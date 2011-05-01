'
' This script is used during project files conversion to the msbuild format
'   - to replace references to projects in the solution file
'   - to fix errors in the wxWidgets project files

Option Explicit

Dim FSO : Set FSO = CreateObject("Scripting.FileSystemObject")
Dim action : action = ""

' Parse our command line arguments
If ParseCommandLine Then
    Select Case action
    Case ""
        Wscript.StdErr.WriteLine "[Error] Invalid action (" & action & ")"

    Case "reference"
        ReplaceReferences

    Case "fix"
        FixErrors

    End Select
End If

Function ReplaceReferences
    FindReplaceInFile "WinSparkleDeps-2010.sln", "wx_vc9_base",  "wx_vc10_base"
    FindReplaceInFile "WinSparkleDeps-2010.sln", "wx_vc9_core",  "wx_vc10_core"
    FindReplaceInFile "WinSparkleDeps-2010.sln", "expat_static", "expat_static-2010"
End Function

Function FixErrors
    FindReplaceInFile "wxWidgets\build\msw\wx_vc10_base.vcxproj", "$(INTDIR) $(OUTDIR);%", "$(INTDIR);$(OUTDIR);%"
    FindReplaceInFile "wxWidgets\build\msw\wx_vc10_core.vcxproj", "$(INTDIR) $(OUTDIR);%", "$(INTDIR);$(OUTDIR);%"
End Function

'////////////////////////////////////////////////////////////////
'// Utilities
'////////////////////////////////////////////////////////////////
Function ParseCommandLine()
    ParseCommandLine = True

    If Wscript.Arguments.Count <> 1 Then
        Wscript.StdErr.WriteLine "[Error] Invalid number of arguments (was: " & Wscript.Arguments.Count & ", expected: 1)"

        ParseCommandLine = False
        Exit Function
    End If

    ' Get the argument
    action = Wscript.Arguments.Item(0)
End Function

Sub FindReplaceInFile(filename, to_find, replacement)
    Dim file, data
    Set file = FSO.OpenTextFile(filename, 1, 0, 0)
    data = file.ReadAll
    file.Close
    data = Replace(data, to_find, replacement)
    Set file = FSO.CreateTextFile(filename, -1, 0)
    file.Write data
    file.Close
End Sub
