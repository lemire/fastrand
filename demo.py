import fastrand

print("generate an integer in [0,1001)")
print(fastrand.pcg32bounded(1001))
print("generate an integer in [100,1000]")
print(fastrand.pcg32randint(100,1000)) # requires Python 3.7 or better
print("Generate a random 32-bit integer.")
print(fastrand.pcg32())

if fastrand.SIXTYFOUR:
    print("generate a 64-bit integer in [0,1001000000000)")
    print(fastrand.xorshift128plusbounded(1001000000000))
    print("generate a 64-bit integer in [100,1000000000000]")
    print(fastrand.xorshift128plusrandint(100,1000000000000)) # requires Python 3.7 or better
    print("Generate a random 64-bit integer.")
    print(fastrand.xorshift128plus())
