# Beating the Nyquist frequency

delta x != x'

F = m·a

F = -k·x -d·x'

F = AB · ( (|AB| - relaxed_length)·k / |AB| ) + AB · ( (AB ● velAB) · spring_d / |AB|^2 )

F = (AB / |AB|) · (|AB| - relaxed_length) · k + (AB / |AB|) · ( (AB / |AB|) ● velAB) · spring_d )


## Express the spring function in terms of the spring direction

spring_dir = AB / |AB| ~= AB_0 / |AB_0|
spring_length = |AB| = AB ● AB / |AB| = AB ● spring_dir

F = spring_dir · (|AB| - relaxed_length) · spring_k + spring_dir · (spring_dir ● velAB) · spring_d)
F = spring_dir · (spring_dir ● AB - relaxed_length) · spring_k + spring_dir · (spring_dir ● velAB) · spring_d)
F = spring_dir · ((spring_dir ● AB - relaxed_length) · spring_k + (spring_dir ● velAB) · spring_d))

## Reformulate in matrix form

F = spring_dir · ((spring_dir ● (B - A) - relaxed_length) · spring_k + (spring_dir ● (B' - A')) · spring_d))

m·A'' = spring_dir · ((spring_dir ● (B - A) - relaxed_length) · spring_k + (spring_dir ● (B' - A')) · spring_d))
m·A'' = spring_dir · spring_k · spring_dir ● B - spring_dir · spring_k · spring_dir ● A - spring_dir · spring_k · relaxed_length + spring_dir · spring_d · spring_dir ● B' - spring_dir · spring_d · spring_dir ● A'

D = spring_dir
m·A'' = D·k·D●B - D·k·D●A - D·k·relaxed_length + D·d·D●B' - D·d·D●A'
m·A'' = -D·k·D●A + D·k·D●B - D·d·D●A' + D·d·D●B' - D·k·relaxed_length

m·A.x'' = -D.x·k·D●A + D.x·k·D●B - D.x·d·D●A' + D.x·d·D●B' - D.x·k·relaxed_length
m·A.y'' = -D.y·k·D●A + D.y·k·D●B - D.y·d·D●A' + D.y·d·D●B' - D.y·k·relaxed_length
m·A.z'' = -D.z·k·D●A + D.z·k·D●B - D.z·d·D●A' + D.z·d·D●B' - D.z·k·relaxed_length

m·A.x'' = -D.x·k·(D.x·A.x + D.y·A.y + D.z·A.z) + D.x·k·(D.x·B.x + D.y·B.y + D.z·B.z) - D.x·d·(D.x·A.x' + D.y·A.y' + D.z·A.z') + D.x·d·(D.x·B.x' + D.y·B.y' + D.z·B.z') - D.x·k·relaxed_length
m·A.y'' = -D.y·k·(D.x·A.x + D.y·A.y + D.z·A.z) + D.y·k·(D.x·B.x + D.y·B.y + D.z·B.z) - D.y·d·(D.x·A.x' + D.y·A.y' + D.z·A.z') + D.y·d·(D.x·B.x' + D.y·B.y' + D.z·B.z') - D.y·k·relaxed_length
m·A.z'' = -D.z·k·(D.x·A.x + D.y·A.y + D.z·A.z) + D.z·k·(D.x·B.x + D.y·B.y + D.z·B.z) - D.z·d·(D.x·A.x' + D.y·A.y' + D.z·A.z') + D.z·d·(D.x·B.x' + D.y·B.y' + D.z·B.z') - D.z·k·relaxed_length

     0    1    2    3    4    5    6     7     8     9     10    11
X = [A.x, A.y, A.z, B.x, B.y, B.z, A.x', A.y', A.z', B.x', B.y', B.z']

X0'  = X6
X1'  = X7
X2'  = X8
X3'  = X9
X4'  = X10
X5'  = X11
X6'  = (1/A.m)·(-D.x·k·(D.x·X0 + D.y·X1 + D.z·X2) + D.x·k·(D.x·X3 + D.y·X4 + D.z·X5) - D.x·d·(D.x·X6 + D.y·X7 + D.z·X8) + D.x·d·(D.x·X9 + D.y·X10 + D.z·X11) - D.x·k·relaxed_length)
X7'  = (1/A.m)·(-D.y·k·(D.x·X0 + D.y·X1 + D.z·X2) + D.y·k·(D.x·X3 + D.y·X4 + D.z·X5) - D.y·d·(D.x·X6 + D.y·X7 + D.z·X8) + D.y·d·(D.x·X9 + D.y·X10 + D.z·X11) - D.y·k·relaxed_length)
X8'  = (1/A.m)·(-D.z·k·(D.x·X0 + D.y·X1 + D.z·X2) + D.z·k·(D.x·X3 + D.y·X4 + D.z·X5) - D.z·d·(D.x·X6 + D.y·X7 + D.z·X8) + D.z·d·(D.x·X9 + D.y·X10 + D.z·X11) - D.z·k·relaxed_length)
X9'  = (1/B.m)·( D.x·k·(D.x·X0 + D.y·X1 + D.z·X2) - D.x·k·(D.x·X3 + D.y·X4 + D.z·X5) + D.x·d·(D.x·X6 + D.y·X7 + D.z·X8) - D.x·d·(D.x·X9 + D.y·X10 + D.z·X11) + D.x·k·relaxed_length)
X10' = (1/B.m)·( D.y·k·(D.x·X0 + D.y·X1 + D.z·X2) - D.y·k·(D.x·X3 + D.y·X4 + D.z·X5) + D.y·d·(D.x·X6 + D.y·X7 + D.z·X8) - D.y·d·(D.x·X9 + D.y·X10 + D.z·X11) + D.y·k·relaxed_length)
X11' = (1/B.m)·( D.z·k·(D.x·X0 + D.y·X1 + D.z·X2) - D.z·k·(D.x·X3 + D.y·X4 + D.z·X5) + D.z·d·(D.x·X6 + D.y·X7 + D.z·X8) - D.z·d·(D.x·X9 + D.y·X10 + D.z·X11) + D.z·k·relaxed_length)
