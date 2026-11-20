print('pointer derefence test')
print()

x = 3.14159
y = x + 299

px = &x
py = &y

i = 11
*px = *px + 1
j = i ** 2

if j > i:
{
  i = 13
  j = i ** 2
  print(i)
  print(j)
}

pi = &i
pj = &j

s = "this is a string, "
ps = &s

a = *px + 99.25
print(a)

if *px == 1.0:
{
  print('then')
}
elif *px == 2.0:
{
  print("elif")
}
else:
{
  print('else')

  if *px > 4.0:
  {
      b = i - *py
      print(b)

      if False:
      {
        print('false is true')
      }
      elif True:
      {
          print('true is indeed true')
      }
      else:
      {
          print('false is indeed false')
      }

      t = *ps + " with more to follow"
      print(t)

      c = *pj * *pi
      print(c)
  }
  else:
  {
      print('should not happen')
  }

  print('else done')
}

d = *px < *py
e = *px >= *py
f = *pj / *pi
print(d)
print(e)
print(f)

print()

print('done')
