#ifndef INCLUDE_MODERNDBS_CODEGEN_EXPRESSION_H
#define INCLUDE_MODERNDBS_CODEGEN_EXPRESSION_H

#include <memory>
#include "moderndbs/codegen/jit.h"
#include <llvm/ExecutionEngine/ExecutionEngine.h>
#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/Module.h>

namespace moderndbs {
    /// A value (can either be a signed (!) 64 bit integer or a double).
    using data64_t = uint64_t;

    struct Expression {

        /// A value type.
        enum class ValueType {
            INT64,
            DOUBLE
        };

        /// The constant type.
        ValueType type;

        /// Constructor.
        explicit Expression(ValueType type): type(type) {}
        /// Destructor
        virtual ~Expression() = default;

        /// Get the expression type.
        ValueType getType() { return type; }
        /// Evaluate the expression.
        virtual data64_t evaluate(const data64_t* args);
        /// Build the expression code.
        virtual llvm::Value* build(llvm::IRBuilder<>& builder, llvm::Value* args);
    };

    struct Constant: public Expression {
        /// The constant value.
        data64_t value;

        /// Constructor.
        Constant(long long value)
            : Expression(ValueType::INT64), value(*reinterpret_cast<data64_t*>(&value)) {}
        /// Constructor.
        Constant(double value)
            : Expression(ValueType::DOUBLE), value(*reinterpret_cast<data64_t*>(&value)) {}

        data64_t evaluate(const data64_t* args) override {
           return this->value;
        }

        // Can either be int or double
        llvm::Value* build(llvm::IRBuilder<>& builder, llvm::Value* args) override {
           if (this->getType() == ValueType::DOUBLE) {
              return llvm::ConstantFP::get(builder.getDoubleTy(), *reinterpret_cast<double *>(&this->value));
           } else {
              return llvm::ConstantInt::get(builder.getInt64Ty(), this->value, true);
           }
        }
    };

    struct Argument: public Expression {
        /// The argument index.
        uint64_t index;

        /// Constructor.
        Argument(uint64_t index, ValueType type)
            : Expression(type), index(index) {}

        /// TODO(students) implement evaluate and build
        data64_t evaluate(const data64_t* args) override {
           return args[this->index];
        }

       llvm::Value* build(llvm::IRBuilder<>& builder, llvm::Value* args) override {
          llvm::Value* argument;

          switch (this->getType()) {
             case Expression::ValueType::DOUBLE:
                argument = builder.CreateGEP(args, builder.getInt64(index));
                return builder.CreateLoad(builder.getDoubleTy(), argument);
             case Expression::ValueType::INT64:
                argument = builder.CreateGEP(args, builder.getInt64(index));
                return builder.CreateLoad(builder.getInt64Ty(), argument);
          }
       }
    };

    struct Cast: public Expression {
        /// The child.
        Expression& child;
        /// The child type.
        ValueType childType;

        /// Constructor.
        Cast(Expression& child, ValueType type)
            : Expression(type), child(child) {
            childType = child.getType();
        }

        /// TODO(students) implement evaluate and build
        data64_t evaluate(const data64_t* args) override {

        }

        llvm::Value* build(llvm::IRBuilder<>& builder, llvm::Value* args) {

        }
    };

    struct BinaryExpression: public Expression {
        /// The left child.
        Expression& left;
        /// The right child.
        Expression& right;

        /// Constructor.
        BinaryExpression(Expression& left, Expression& right)
            : Expression(ValueType::INT64), left(left), right(right) {
            assert(left.getType() == right.getType() && "the left and right type must equal");
            type = left.getType();
        }
    };

    struct AddExpression: public BinaryExpression {
        /// Constructor
        AddExpression(Expression& left, Expression& right)
            : BinaryExpression(left, right) {}

        data64_t evaluate(const data64_t* args) override {
           auto leftE = this->left.evaluate(args);
           auto rightE = this->right.evaluate(args);
           switch (this->getType()) {
              case Expression::ValueType::INT64:
                 return leftE + rightE;
              case Expression::ValueType::DOUBLE:
                 double result = *reinterpret_cast<double*>(&leftE) + *reinterpret_cast<double*>(&rightE);
                 return *reinterpret_cast<data64_t*>(&result);
           }
        }

        llvm::Value* build(llvm::IRBuilder<>& builder, llvm::Value* args) {
           llvm::Value* valueLeft = this->left.build(builder, args);
           llvm::Value* valueRight = this->right.build(builder, args);
           llvm::Value* addition;
           switch (this->getType()) {
              case Expression::ValueType::INT64:
                 addition = builder.CreateAdd(valueLeft, valueRight);
                 break;
              case Expression::ValueType::DOUBLE:
                 // here
                 addition = builder.CreateFAdd(valueLeft, valueRight);
                 break;
           }
           return addition;
        }
    };

    struct SubExpression: public BinaryExpression {
        /// Constructor
        SubExpression(Expression& left, Expression& right)
            : BinaryExpression(left, right) {}

        /// TODO(students) implement evaluate and build
        data64_t evaluate(const data64_t* args) override {
           auto leftE = this->left.evaluate(args);
           auto rightE = this->right.evaluate(args);
           switch (this->getType()) {
              case Expression::ValueType::INT64:
                 return leftE - rightE;
              case Expression::ValueType::DOUBLE:
                 double result = *reinterpret_cast<double*>(&leftE) - *reinterpret_cast<double*>(&rightE);
                 return *reinterpret_cast<data64_t*>(&result);
           }
        }

        llvm::Value* build(llvm::IRBuilder<>& builder, llvm::Value* args) {
           llvm::Value* valueLeft = this->left.build(builder, args);
           llvm::Value* valueRight = this->right.build(builder, args);
           llvm::Value* subE;
           switch (this->getType()) {
              case Expression::ValueType::INT64:
                 subE = builder.CreateSub(valueLeft, valueRight);
                 break;
              case Expression::ValueType::DOUBLE:
                 subE = builder.CreateFSub(valueLeft, valueRight);
                 break;
           }
           return subE;
        }
    };

    struct MulExpression: public BinaryExpression {
        /// Constructor
        MulExpression(Expression& left, Expression& right)
            : BinaryExpression(left, right) {}

        /// TODO(students) implement evaluate and build
        data64_t evaluate(const data64_t* args) override {
           auto leftE = this->left.evaluate(args);
           auto rightE = this->right.evaluate(args);
           switch (this->getType()) {
              case Expression::ValueType::INT64:
                 return leftE * rightE;
              case Expression::ValueType::DOUBLE:
                 // one is a multiplication one is a pointer :)
                 double result = *reinterpret_cast<double*>(&leftE) * *reinterpret_cast<double*>(&rightE);
                 return *reinterpret_cast<data64_t*>(&result);
           }
        }

        llvm::Value* build(llvm::IRBuilder<>& builder, llvm::Value* args) {
           llvm::Value* valueLeft = this->left.build(builder, args);
           llvm::Value* valueRight = this->right.build(builder, args);
           llvm::Value* multiplication;
           switch (this->getType()) {
              case Expression::ValueType::INT64:
                 multiplication = builder.CreateMul(valueLeft, valueRight);
                 break;
              case Expression::ValueType::DOUBLE:
                 multiplication = builder.CreateFMul(valueLeft, valueRight);
                 break;
           }
           return multiplication;
        }
    };

    struct DivExpression: public BinaryExpression {
        /// Constructor
        DivExpression(Expression& left, Expression& right)
            : BinaryExpression(left, right) {}

        /// TODO(students) implement evaluate and build
        data64_t evaluate(const data64_t* args) override {
           auto leftE = this->left.evaluate(args);
           auto rightE = this->right.evaluate(args);
           switch (this->getType()) {
              case Expression::ValueType::INT64:
                 return leftE / rightE;
              case Expression::ValueType::DOUBLE:
                 double result = *reinterpret_cast<double*>(&leftE) / *reinterpret_cast<double*>(&rightE);
                 return *reinterpret_cast<data64_t*>(&result);
           }
        }

       llvm::Value* build(llvm::IRBuilder<>& builder, llvm::Value* args) {
          llvm::Value* valueLeft = this->left.build(builder, args);
          llvm::Value* valueRight = this->right.build(builder, args);
          llvm::Value* division;
          switch (this->getType()) {
             case Expression::ValueType::INT64:
                division = builder.CreateSDiv(valueLeft, valueRight);
                break;
             case Expression::ValueType::DOUBLE:
                division = builder.CreateFDiv(valueLeft, valueRight);
                break;
          }
          return division;
       }
    };

    struct ExpressionCompiler {
        /// The llvm context.
        llvm::orc::ThreadSafeContext& context;
        /// The llvm module.
        std::unique_ptr<llvm::Module> module;
        /// The jit.
        JIT jit;
        /// The compiled function.
        data64_t (*fnPtr)(data64_t* args);

        /// Constructor.
        explicit ExpressionCompiler(llvm::orc::ThreadSafeContext& context);

        /// Compile an expression.
        void compile(Expression& expression, bool verbose = false);
        /// Run a previously compiled expression
        data64_t run(data64_t* args);
    };

}  // namespace moderndbs

#endif
