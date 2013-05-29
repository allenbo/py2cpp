class C:
    x = 0
    def dosomething(self):
        print 'C'

class D:
    x = 1
    def dosomething(self):
        print 'D'

class E(D, C):
    x = 2

E().dosomething()
