Player = {
    Name = "Xexus",
    Title = "Duke of Cubines",
    Level = 40,
    Stuff = "Hello",
    Fun = function(input)
        print("Hello from table function. Got input", input)
        return input + 10, 3.14
    end
}

print(super.answer)
print(super.f(1, 2))

function AddStuff(a, b)
    print("ADDSTUFF")
    return a * b + 2
end

function DoAThing(a, b)
    print("Do a thing")
    return HostFunction(a + 10, b + 15)
end
