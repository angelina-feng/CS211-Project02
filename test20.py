#
# stmt within if-then has semantic error
#
print('starting')

x = 10

while x > 0:
{
  print(x)

  if x >= 5:
  {
    print("greater than, or equal to, 5")
    x = x - "oops"  ## semantic error
  }

  x = x - 1
}

print('done')
