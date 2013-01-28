@echo off

Set PATH=D:\Program Files (x86)\Microsoft Visual Studio 8\Common7\IDE;D:\Program Files (x86)\Microsoft Visual Studio 8\VC\bin;%PATH%
Set INCLUDE=D:\Program Files (x86)\Microsoft Visual Studio 8\VC\include;%INCLUDE%
Set LIB=D:\Program Files (x86)\Microsoft Visual Studio 8\VC;%LIB%

Set FILE1=Atoms/GFFreeable
Set FILE2=Molecules/GFString
Set FILE3=Molecules/GFVector
Set FILE4=Materials/GFFunctions

cl /c %FILE1%.cpp /Fo%FILE1%.obj
cl /c %FILE2%.cpp /Fo%FILE2%.obj
cl /c %FILE3%.cpp /Fo%FILE3%.obj
cl /c %FILE4%.cpp /Fo%FILE4%.obj


link /lib /out:Groundfloor.lib %FILE1%.obj %FILE2%.obj %FILE3%.obj %FILE4%.obj

