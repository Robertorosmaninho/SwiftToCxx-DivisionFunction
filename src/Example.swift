@_expose(Cxx)
public enum DivByZero : Error {
    case divisorIsZero
    case bothAreZero

    public func getMessage() {
        print(self)
    }
}

@_expose(Cxx)
public func division(_ a: Int, _ b: Int) throws -> Float {
    if a == 0 && b == 0 {
        throw DivByZero.bothAreZero
    } else if b == 0 {
        throw DivByZero.divisorIsZero
    } else {
        return Float(a / b)
    }
}