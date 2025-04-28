#include "val.h"
#include <cmath>

// Numeric addition: only same-type integers or same-type reals
Value Value::operator+(const Value& op) const {
    if (IsInt() && op.IsInt()) {
        return Value(GetInt() + op.GetInt());
    }
    if (IsReal() && op.IsReal()) {
        return Value(GetReal() + op.GetReal());
    }
    cout << "Run-Time Error: Illegal operands for +" << endl;

    return Value();
}

// Numeric subtraction
Value Value::operator-(const Value& op) const {
    if (IsInt() && op.IsInt()) {
        return Value(GetInt() - op.GetInt());
    }
    if (IsReal() && op.IsReal()) {
        return Value(GetReal() - op.GetReal());
    }
    cout << "Run-Time Error: Illegal operands for -" << endl;
    return Value();
}

// Numeric multiplication
Value Value::operator*(const Value& op) const {
    if (IsInt() && op.IsInt()) {
        return Value(GetInt() * op.GetInt());
    }
    if (IsReal() && op.IsReal()) {
        return Value(GetReal() * op.GetReal());
    }
    cout << "Run-Time Error: Illegal operands for *" << endl;
    return Value();
}

// Numeric division
Value Value::operator/(const Value& op) const {
    if (IsInt() && op.IsInt()) {
        int rhs = op.GetInt();
        if (rhs == 0) {
            cout << "Run-Time Error: Division by zero" << endl;
            return Value();
        }
        return Value(GetInt() / rhs);
    }
    if (IsReal() && op.IsReal()) {
        double rhs = op.GetReal();
        if (rhs == 0.0) {
            cout << "Run-Time Error: Division by zero" << endl;
            return Value();
        }
        
        return Value(GetReal() / rhs);
    }
    cout << "Run-Time Error: Illegal operands for /" << endl;
    return Value();
}

// Integer remainder
Value Value::operator%(const Value& op) const {
    if (IsInt() && op.IsInt()) {
        int rhs = op.GetInt();
        if (rhs == 0) 
        {cout << "Run-Time Error: Division by zero" << endl;
        return Value();}
        return Value(GetInt() % rhs);
    }
    cout << "Run-Time Error: Illegal operands for %" << endl;
    return Value();
}

// Equality
Value Value::operator==(const Value& op) const {
    if (T == op.T) {
        switch (T) {
            case VINT:    return Value(GetInt() == op.GetInt());
            case VREAL:   return Value(GetReal() == op.GetReal());
            case VBOOL:   return Value(GetBool() == op.GetBool());
            case VCHAR:   return Value(GetChar() == op.GetChar());
            case VSTRING: return Value(GetString() == op.GetString());
            default: break;
        }
    }
    cout << "Run-Time Error: Illegal operands for ==" << endl;
    return Value();
}

// Inequality
Value Value::operator!=(const Value& op) const {
    if (T == op.T) {
        switch (T) {
            case VINT:    return Value(GetInt() != op.GetInt());
            case VREAL:   return Value(GetReal() != op.GetReal());
            case VBOOL:   return Value(GetBool() != op.GetBool());
            case VCHAR:   return Value(GetChar() != op.GetChar());
            case VSTRING: return Value(GetString() != op.GetString());
            default: break;
        }
    }
    cout << "Run-Time Error: Illegal operands for !=" << endl;
    return Value();
}

// Greater than
Value Value::operator>(const Value& op) const {
    if (IsInt() && op.IsInt()) {
        return Value(GetInt() > op.GetInt());
    }
    if (IsReal() && op.IsReal()) {
        return Value(GetReal() > op.GetReal());
    }
    cout << "Run-Time Error: Illegal operands for >" << endl;
    return Value();
}

// Less than
Value Value::operator<(const Value& op) const {
    if (IsInt() && op.IsInt()) {
        return Value(GetInt() < op.GetInt());
    }
    if (IsReal() && op.IsReal()) {
        return Value(GetReal() < op.GetReal());
    }
    cout << "Run-Time Error: Illegal operands for <" << endl;
    return Value();
}

// Less than or equal
Value Value::operator<=(const Value& op) const {
    Value lt = (*this < op);
    Value eq = (*this == op);
    if (lt.IsErr() || eq.IsErr()) {
        cout << "Run-Time Error: Illegal operands for <=" << endl;
        return Value();
    }
    return Value(lt.GetBool() || eq.GetBool());
}

// Greater than or equal
Value Value::operator>=(const Value& op) const {
    Value gt = (*this > op);
    Value eq = (*this == op);
    if (gt.IsErr() || eq.IsErr()) {
        cout << "Run-Time Error: Illegal operands for >=" << endl;
        return Value();
    }
    return Value(gt.GetBool() || eq.GetBool());
}

// Logical AND
Value Value::operator&&(const Value& op) const {
    if (IsBool() && op.IsBool()) {
        return Value(GetBool() && op.GetBool());
    }
    cout << "Run-Time Error: Illegal operands for &&" << endl;
    return Value();
}

// Logical OR
Value Value::operator||(const Value& op) const {
    if (IsBool() && op.IsBool()) {
        return Value(GetBool() || op.GetBool());
    }
    cout << "Run-Time Error: Illegal operands for ||" << endl;
    return Value();
}

// Logical NOT
Value Value::operator!() const {
    if (IsBool()) {
        return Value(!GetBool());
    }
    cout << "Run-Time Error: Illegal operands for !" << endl;
    return Value();
}

// String/char concatenation
Value Value::Concat(const Value& op) const {
    if ((IsString() || IsChar()) && (op.IsString() || op.IsChar())) {
        string a = IsString() ? GetString() : string(1, GetChar());
        string b = op.IsString() ? op.GetString() : string(1, op.GetChar());
        return Value(a + b);
    }
    cout << "Run-Time Error: Illegal operands for Concat" << endl;
    return Value();
}

// Exponentiation on floats only
Value Value::Exp(const Value& op) const {
    if (IsReal() && op.IsReal()) {
        double base = GetReal();
        double exponent = op.GetReal();
        if (exponent == 0.0) return Value(1.0);
        if (base == 0.0) return Value(0.0);
        return Value(std::pow(base, exponent));
    }
    cout << "Run-Time Error: Illegal operands for Exp" << endl;
    return Value();
}

