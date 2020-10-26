import test

ret = test.add_5(3)
test.crazy_function(True, 'r', -1, 10, -1, 40, -1, 960, -1, 45678, -1, 3.423, -546.231, ret)
test.Print()
test.Print2((123,), "a string coming right up!")

vec = test.Vec(10, 20, 30)

test.f(vec.add(test.Vec(4, 5, 6)))

result = test.g(9)
test.h(vec.x)
test.h(vec.y)
test.h(vec.z)
test.f(result)