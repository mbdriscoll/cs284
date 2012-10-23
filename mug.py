M = 1.0  # big dimension of cup
m = 0.05 # width of lip
w = 0.10 # width of handle

vertices = [

 # outer
 (0, 0, 0),
 (M, 0, 0),
 (0, 0, M),
 (M, 0, M),
 (0, M, 0),
 (M, M, 0),
 (0, M, M),
 (M, M, M),

 # top
 (m, M, M-m),
 (M-m, M, M-m),
 (M-m, M, m),
 (m, M, m),

 # inner
 (m, m, M-m),
 (M-m, m, M-m),
 (M-m, m, m),
 (m, m, m),
]

faces = [
 # outer
 (6, 5, 1),
 (6, 1, 2),
 (8, 6, 2),
 (8, 2, 4),
 (7, 8, 4),
 (7, 4, 3),
 (3, 4, 2),
 (3, 2, 1),

 # top
 (9, 7, 5),
 (9, 5, 12),
 (12, 5, 6),
 (11, 12, 6),
 (8, 11, 6),
 (8, 10, 11),
 (8, 7, 10),
 (10, 7, 9),

 # inner
 (10, 9, 13),
 (10, 13, 14),
 (10, 14, 11),
 (11, 14, 15),
 (11, 15, 12),
 (15, 16, 12),
 (12, 16, 9),
 (16, 13, 9),
 (14, 13, 16),
 (14, 16, 15),

 # handle temp
 (5, 7, 3),
 (5, 3, 1),
]

for point in vertices:
    print "v %f %f %f" % point

for face in faces:
    print "f %d %d %d" % (face[0], face[1], face[2])
