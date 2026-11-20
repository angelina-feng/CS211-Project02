#
# while condition has semantic error
#
print('starting')

x = 10

while x > "banana":  # semantic error: comparing integer and string
{
  print(x)

  if x >= "apple":  # semantic error: comparing integer and string
  {
    print("greater than, or equal to, 5")
  }

  x = x - 1
}

print('done')
