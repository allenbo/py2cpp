x = 3
y = 2
def test(a, b):
  x = a
  y = b
  print 'inside function test, x =', a, 'y =', b

print 'outside function test x =', x, 'y =', y
test(10, 12)
