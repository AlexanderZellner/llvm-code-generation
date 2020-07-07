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

        /// TODO(students) implement evaluate and build
    };

    struct Argument: public Expression {
        /// The argument index.
        uint64_t index;

        /// Constructor.
        Argument(uint64_t index, ValueType type)
            : Expression(type), index(index) {}

        /// TODO(students) implement evaluate and build
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

        /// TODO(students) implement evaluate and build
    };

    struct SubExpression: public BinaryExpression {
        /// Constructor
        SubExpression(Expression& left, Expression& right)
            : BinaryExpression(left, right) {}

        /// TODO(students) implement evaluate and build
    };

    struct MulExpression: public BinaryExpression {
        /// Constructor
        MulExpression(Expression& left, Expression& right)
            : BinaryExpression(left, right) {}

        /// TODO(students) implement evaluate and build
    };

    struct DivExpression: public BinaryExpression {
        /// Constructor
        DivExpression(Expression& left, Expression& right)
            : BinaryExpression(left, right) {}

        /// TODO(students) implement evaluate and build
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
