class New:
    def __init__(self, x):
        self.x = x
    def add(self, a):
        self.x += a
    def printout(self):
        print self.x

n = New(1);
n.add(2)
n.printout()
