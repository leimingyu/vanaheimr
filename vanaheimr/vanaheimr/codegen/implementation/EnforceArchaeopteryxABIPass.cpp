/*! \file   EnforceArchaeopteryxABIPass.cpp
	\author Gregory Diamos <gdiamos@nvidia.com>
	\date   Friday December 21, 2021
	\brief  The source file for the EnforceArchaeopteryxABIPass class.
*/

// Vanaheimr Includes
#include <vanaheimr/codegen/interface/EnforceArchaeopteryxABIPass.h>

#include <vanaheimr/abi/interface/ApplicationBinaryInterface.h>

#include <vanaheimr/ir/interface/Module.h>
#include <vanaheimr/ir/interface/Type.h>

#include <vanaheimr/util/interface/LargeMap.h>
#include <vanaheimr/util/interface/SmallMap.h>

// Hydrazine Includes
#include <hydrazine/interface/debug.h>

// Preprocessor Macros
#ifdef REPORT_BASE
#undef REPORT_BASE
#endif

#define REPORT_BASE 1

namespace vanaheimr
{

namespace codegen
{

EnforceArchaeopteryxABIPass::EnforceArchaeopteryxABIPass()
: ModulePass({}, "EnforceArchaeopteryxABIPass")
{

}

typedef util::LargeMap<std::string, uint64_t> GlobalToAddressMap;
typedef util::SmallMap<std::string, uint64_t>  LocalToAddressMap;

static void layoutGlobals(ir::Module& module, GlobalToAddressMap& globals,
	const abi::ApplicationBinaryInterface& abi);
static void layoutLocals(ir::Function& function, LocalToAddressMap& globals,
	const abi::ApplicationBinaryInterface& abi);
static void lowerFunction(ir::Function& function,
	const abi::ApplicationBinaryInterface& abi,
	const GlobalToAddressMap& globals, const LocalToAddressMap& locals);

static const abi::ApplicationBinaryInterface* getABI();

void EnforceArchaeopteryxABIPass::runOnModule(Module& m)
{
	report("Lowering " << m.name << " to target the archaeopteryx ABI.");

	const abi::ApplicationBinaryInterface* abi = getABI();

	GlobalToAddressMap globals;

	layoutGlobals(m, globals, *abi);
	
	// barrier
	report(" Lowering functions...");
	
	// For-all
	for(auto function = m.begin(); function != m.end(); ++function)
	{
		LocalToAddressMap locals;
	
		layoutLocals(*function, locals, *abi);
		
		// barrier

		lowerFunction(*function, *abi, globals, locals);
	}
}

static unsigned int align(unsigned int address, unsigned int alignment)
{
	unsigned int remainder = address % alignment;
	unsigned int offset = remainder == 0 ? 0 : alignment - remainder;
	
	return address + offset;	
}

static void layoutGlobals(ir::Module& module, GlobalToAddressMap& globals,
	const abi::ApplicationBinaryInterface& abi)
{
	unsigned int offset = 0;

	report(" Lowering globals...");

	for(auto global = module.global_begin();
		global != module.global_end(); ++global)
	{
		offset = align(offset, global->type().alignment());
		
		report("  Laying out '" << global->name() << "' at " << offset);
		
		globals.insert(std::make_pair(global->name(), offset));

		offset += global->bytes();
	}
}

static void layoutLocals(ir::Function& function, LocalToAddressMap& locals,
	const abi::ApplicationBinaryInterface& abi)
{
	assertM(function.local_empty(), "Lowering locals not implemented.");
}

static void lowerCall(ir::Instruction* i,
	const abi::ApplicationBinaryInterface& abi)
{
	assertM(false, "call lowering not implemented.");
}

static void lowerIntrinsic(ir::Instruction* i,
	const abi::ApplicationBinaryInterface& abi)
{
	// This is a no-op
}

static void lowerReturn(ir::Instruction* i,
	const abi::ApplicationBinaryInterface& abi)
{
	if(i->block->function()->hasAttribute("kernel")) return;

	assertM(false, "function return lowering not implemented.");
}

static void lowerAddress(ir::Operand*& read, const GlobalToAddressMap& globals,
	const LocalToAddressMap& locals)
{
	auto variableRead = static_cast<ir::AddressOperand*>(read);

	auto local = locals.find(variableRead->globalValue->name());
	
	if(local != locals.end())
	{
		auto immediate = new ir::ImmediateOperand(local->second,
			read->instruction(), read->type());

		read = immediate;

		delete variableRead;
		return;
	}

	auto global = globals.find(variableRead->globalValue->name());

	assertM(global != globals.end(), "'" << variableRead->globalValue->name()
		<< "' not lowered correctly.");

	auto immediate = new ir::ImmediateOperand(global->second,
		read->instruction(), read->type());

	read = immediate;

	delete variableRead;

}

static void lowerEntryPoint(ir::Function& function, 
	const abi::ApplicationBinaryInterface& abi)
{
	// kernels don't need explicit entry point code
	if(function.hasAttribute("kernel")) return;

	assertM(false, "Entry point handling for called "
		"functions is not implemented yet");
}

static void lowerFunction(ir::Function& function,
	const abi::ApplicationBinaryInterface& abi,
	const GlobalToAddressMap& globals, const LocalToAddressMap& locals)
{
	if(function.isIntrinsic()) return;

	report("  Lowering function '" << function.name() << "'");
	
	// add an entry point
	lowerEntryPoint(function, abi);

	// for all 
	for(auto block = function.begin(); block != function.end(); ++block)
	{
		for(auto instruction : *block)
		{
			// lower calls
			if(instruction->isCall())
			{
				if(instruction->isIntrinsic())
				{
					lowerIntrinsic(instruction, abi);
					continue;
				}
				else
				{
					lowerCall(instruction, abi);
					continue;
				}
			}
			
			// lower returns
			if(instruction->isReturn())
			{
				lowerReturn(instruction, abi);
				continue;
			}

			// lower variable accesses
			for(auto read : instruction->reads)
			{
				if(read->isAddress())
				{
					if(read->isBasicBlock()) continue;
					
					lowerAddress(read, globals, locals);
				}
			}
		}
	}
}

static const abi::ApplicationBinaryInterface* getABI()
{
	auto archaeopteryxABI =
		abi::ApplicationBinaryInterface::getABI("archaeopteryx");

	return archaeopteryxABI;
}

}

}
