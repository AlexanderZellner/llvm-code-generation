#include "moderndbs/codegen/expression.h"
#include "moderndbs/error.h"
#include <iostream>

using JIT = moderndbs::JIT;
using Expression = moderndbs::Expression;
using ExpressionCompiler = moderndbs::ExpressionCompiler;
using data64_t = moderndbs::data64_t;

/// Evaluate the expresion.
data64_t Expression::evaluate(const data64_t* /*args*/) {
   throw NotImplementedException();
}

/// Build the expression
llvm::Value* Expression::build(llvm::IRBuilder<>& /*builder*/, llvm::Value* /*args*/) {
   throw NotImplementedException();
}

/// Constructor.
ExpressionCompiler::ExpressionCompiler(llvm::orc::ThreadSafeContext& context)
   : context(context), module(std::make_unique<llvm::Module>("meaningful_module_name", *context.getContext())), jit(context), fnPtr(nullptr) {}

/// Compile an expression.
void ExpressionCompiler::compile(Expression& expression, bool verbose) {
   llvm::IRBuilder<> builder(*this->context.getContext());
   auto printfT = llvm::FunctionType::get(llvm::Type::getInt32Ty(*this->context.getContext()), {llvm::Type::getInt8Ty(*this->context.getContext())}, true);
   auto printfFn = llvm::cast<llvm::Function>(this->module->getOrInsertFunction("printf", printfT).getCallee());

   auto function_type = llvm::FunctionType::get(builder.getInt64Ty(), {llvm::PointerType::get(llvm::Type::getInt64Ty(*this->context.getContext()), 0)}, false);

   //auto function_type = llvm::FunctionType::get(llvm::Type::getInt64Ty(*this->context.getContext()), {llvm::PointerType::get(llvm::Type::getInt64Ty(*this->context.getContext()),0)}, false);
   auto function = llvm::cast<llvm::Function>(this->module->getOrInsertFunction("function", function_type).getCallee());

   llvm::BasicBlock *basicBlock = llvm::BasicBlock::Create(*this->context.getContext(), "entry", function);
   builder.SetInsertPoint(basicBlock);

   std::vector<llvm::Value *> fooFnArgs;
   for(llvm::Function::arg_iterator ai = function->arg_begin(), ae = function->arg_end(); ai != ae; ++ai) {
      fooFnArgs.push_back(&*ai);
   }

   llvm::Value* value = expression.build(builder, fooFnArgs[0]);
   //llvm::Value *fooFnStrPtr = builder.CreateGlobalStringPtr("%d\n", "f");
   //std::vector<llvm::Value*> fprintArgs;
   //fprintArgs.push_back(fooFnStrPtr);
   auto result = builder.CreateBitCast(value, builder.getInt64Ty());
   //fprintArgs.push_back(result);

   //builder.CreateCall(printfFn, llvm::makeArrayRef(fprintArgs));

   builder.CreateRet(result);
   //this->module->print(llvm::errs(), nullptr);
   auto module = this->jit.addModule(std::move(this->module));
   void* ptr_function = jit.getPointerToFunction("function");
   this->fnPtr = reinterpret_cast<data64_t (*) (data64_t*)>(ptr_function);
}

/// Compile an expression.
data64_t ExpressionCompiler::run(data64_t* args) {
   return fnPtr(args);
}
