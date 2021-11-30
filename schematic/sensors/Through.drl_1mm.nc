(G-CODE GENERATED BY FLATCAM v8.994 - www.flatcam.org - Version Date: 2020/11/7)

(Name: Through.drl_edit_cnc)
(Type: G-code from Geometry)
(Units: MM)

(Created on Wednesday, 24 November 2021 at 13:51)

(This preprocessor is used with a motion controller loaded with GRBL firmware.)
(It is configured to be compatible with almost any version of GRBL firmware.)


(TOOLS DIAMETER: )
(Tool: 1 -> Dia: 0.5994)
(Tool: 2 -> Dia: 1.0008)
(Tool: 3 -> Dia: 2.9972)

(FEEDRATE Z: )
(Tool: 1 -> Feedrate: 300)
(Tool: 2 -> Feedrate: 300)
(Tool: 3 -> Feedrate: 300)

(FEEDRATE RAPIDS: )
(Tool: 1 -> Feedrate Rapids: 1500)
(Tool: 2 -> Feedrate Rapids: 1500)
(Tool: 3 -> Feedrate Rapids: 1500)

(Z_CUT: )
(Tool: 1 -> Z_Cut: -1.7)
(Tool: 2 -> Z_Cut: -1.7)
(Tool: 3 -> Z_Cut: -1.7)

(Tools Offset: )
(Tool: 2 -> Offset Z: 0.0)

(Z_MOVE: )
(Tool: 1 -> Z_Move: 2)
(Tool: 2 -> Z_Move: 2)
(Tool: 3 -> Z_Move: 2)

(Z Toolchange: 15 mm)
(X,Y Toolchange: 0.0000, 0.0000 mm)
(Z Start: None mm)
(Z End: 0.5 mm)
(X,Y End: None mm)
(Steps per circle: 64)
(Steps per circle: 64)
(Preprocessor Excellon: GRBL_11_no_M6)

(X range:    3.9728 ...   32.9237  mm)
(Y range:    6.1214 ...    9.7079  mm)

(Spindle Speed: 10000 RPM)
G21
G90
G17
G94


G01 F300.00

M5             
G00 Z15.0000
G00 X0.0000 Y0.0000                
T2
(MSG, Change to Tool Dia = 1.0008 ||| Total drills for tool T2 = 4)
M0
G00 Z15.0000
        
G01 F300.00
M03 S10000
G00 X4.4732 Y9.2075
G01 Z-1.7000
G01 Z0
G00 Z2.0000
G00 X4.4732 Y6.6675
G01 Z-1.7000
G01 Z0
G00 Z2.0000
G00 X9.9748 Y6.6675
G01 Z-1.7000
G01 Z0
G00 Z2.0000
G00 X9.9748 Y9.2075
G01 Z-1.7000
G01 Z0
G00 Z2.0000
M05
G00 Z0.50

