import pytct

delta = [(0,11,1),
         (1,10,0),
         (1,12,2),
         (2,14,3),
         (2,13,0),
         (0,15,4)]
Qm = [0,1]
pytct.create("test", 5, delta, Qm)

disp = pytct.AutomatonDisplay("test")

disp.render()

disp.save("test", "jpg")