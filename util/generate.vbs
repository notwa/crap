Set FSO = CreateObject("Scripting.FileSystemObject")

Name = WScript.Arguments(0)
Kind = WScript.Arguments(1)

IPath = "../template/"&Kind&".cpp"
OPath = "../crap/"&Kind&"/"&Name&".cpp"
Inc = "crap/"&Name&".hpp"

Set File = FSO.OpenTextFile(IPath)
Text = File.ReadAll
File.Close

Text = Replace(Text, "//#INCLUDE", "#include """&Inc&"""")
Text = Replace(Text, "//#REDEFINE", "#define CrapPlug Crap_"&Name)

Set File = FSO.CreateTextFile(OPath, True)
File.Write Text
File.Close
