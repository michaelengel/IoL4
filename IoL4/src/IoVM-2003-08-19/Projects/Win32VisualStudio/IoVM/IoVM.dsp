# Microsoft Developer Studio Project File - Name="IoVM" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Console Application" 0x0103

CFG=IoVM - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "IoVM.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "IoVM.mak" CFG="IoVM - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "IoVM - Win32 Release" (based on "Win32 (x86) Console Application")
!MESSAGE "IoVM - Win32 Debug" (based on "Win32 (x86) Console Application")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "IoVM - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release"
# PROP Intermediate_Dir "Release"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_CONSOLE" /D "_MBCS" /YX /FD /c
# ADD CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_CONSOLE" /D "_MBCS" /YX /FD /c
# ADD BASE RSC /l 0x40c /d "NDEBUG"
# ADD RSC /l 0x40c /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib  kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /machine:I386
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib  kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /machine:I386

!ELSEIF  "$(CFG)" == "IoVM - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debug"
# PROP Intermediate_Dir "Debug"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_CONSOLE" /D "_MBCS" /YX /FD /GZ  /c
# ADD CPP /nologo /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_CONSOLE" /D "_MBCS" /D strncasecmp=_strnicmp /YX /FD /GZ  /c
# ADD BASE RSC /l 0x40c /d "_DEBUG"
# ADD RSC /l 0x40c /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib  kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /debug /machine:I386 /pdbtype:sept
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib  kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /debug /machine:I386 /pdbtype:sept

!ENDIF 

# Begin Target

# Name "IoVM - Win32 Release"
# Name "IoVM - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Group "Base"

# PROP Default_Filter "*.c"
# Begin Source File

SOURCE=..\..\..\IoVM\base\ByteArray.c
# End Source File
# Begin Source File

SOURCE=..\..\..\IoVM\base\CLI.c
# End Source File
# Begin Source File

SOURCE=..\..\..\IoVM\base\Hash.c
# End Source File
# Begin Source File

SOURCE=..\..\..\IoVM\base\LinkedList.c
# End Source File
# Begin Source File

SOURCE=..\..\..\IoVM\base\List.c
# End Source File
# Begin Source File

SOURCE=..\..\..\IoVM\base\MainArgs.c
# End Source File
# Begin Source File

SOURCE=..\..\..\IoVM\base\Parser.c
# End Source File
# Begin Source File

SOURCE=..\..\..\IoVM\base\PHash.c
# End Source File
# Begin Source File

SOURCE=..\..\..\IoVM\base\PortableStrptime.c
# End Source File
# Begin Source File

SOURCE=..\..\..\IoVM\base\Scheduler.c
# End Source File
# Begin Source File

SOURCE=..\..\..\IoVM\base\SplayTree.c
# End Source File
# Begin Source File

SOURCE=..\..\..\IoVM\base\Stack.c
# End Source File
# End Group
# Begin Source File

SOURCE=..\..\..\IoVM\IoBlock.c
# End Source File
# Begin Source File

SOURCE=..\..\..\IoVM\IoBuffer.c
# End Source File
# Begin Source File

SOURCE=..\..\..\IoVM\IoCFunction.c
# End Source File
# Begin Source File

SOURCE=..\..\..\IoVM\IoCoroutine.c
# End Source File
# Begin Source File

SOURCE=..\..\..\IoVM\IoDate.c
# End Source File
# Begin Source File

SOURCE=..\..\..\IoVM\IoDuration.c
# End Source File
# Begin Source File

SOURCE=..\..\..\IoVM\IoException.c
# End Source File
# Begin Source File

SOURCE=..\..\..\IoVM\IoExceptionCatch.c
# End Source File
# Begin Source File

SOURCE=..\..\..\IoVM\IoFile.c
# End Source File
# Begin Source File

SOURCE=..\..\..\IoVM\IoJITFunction.c
# End Source File
# Begin Source File

SOURCE=..\..\..\IoVM\IoLinkedList.c
# End Source File
# Begin Source File

SOURCE=..\..\..\IoVM\IoList.c
# End Source File
# Begin Source File

SOURCE=..\..\..\IoVM\IoMap.c
# End Source File
# Begin Source File

SOURCE=..\..\..\IoVM\IoMessage.c
# End Source File
# Begin Source File

SOURCE=..\..\..\IoVM\IoMessage_parser.c
# End Source File
# Begin Source File

SOURCE=..\..\..\IoVM\IoNil.c
# End Source File
# Begin Source File

SOURCE=..\..\..\IoVM\IoNumber.c
# End Source File
# Begin Source File

SOURCE=..\..\..\IoVM\IoObject.c
# End Source File
# Begin Source File

SOURCE=..\..\..\IoVM\IoObject_actor.c
# End Source File
# Begin Source File

SOURCE=..\..\..\IoVM\IoState.c
# End Source File
# Begin Source File

SOURCE=..\..\..\IoVM\IoString.c
# End Source File
# Begin Source File

SOURCE=..\..\..\IoVM\IoTag.c
# End Source File
# Begin Source File

SOURCE=..\..\..\IoVM\IoValue.c
# End Source File
# Begin Source File

SOURCE=..\..\..\IoVM\IoValueGroup.c
# End Source File
# Begin Source File

SOURCE=..\..\..\IoVM\IoWeakLink.c
# End Source File
# Begin Source File

SOURCE=..\..\..\IoVM\main.c
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=..\..\..\IoVM\base\ByteArray.h
# End Source File
# Begin Source File

SOURCE=..\..\..\IoVM\base\CLI.h
# End Source File
# Begin Source File

SOURCE=..\..\..\IoVM\base\Common.h
# End Source File
# Begin Source File

SOURCE=..\..\..\IoVM\base\Hash.h
# End Source File
# Begin Source File

SOURCE=..\..\..\IoVM\IoBlock.h
# End Source File
# Begin Source File

SOURCE=..\..\..\IoVM\IoBuffer.h
# End Source File
# Begin Source File

SOURCE=..\..\..\IoVM\IoCFunction.h
# End Source File
# Begin Source File

SOURCE=..\..\..\IoVM\IoCoroutine.h
# End Source File
# Begin Source File

SOURCE=..\..\..\IoVM\IoDate.h
# End Source File
# Begin Source File

SOURCE=..\..\..\IoVM\IoDuration.h
# End Source File
# Begin Source File

SOURCE=..\..\..\IoVM\IoException.h
# End Source File
# Begin Source File

SOURCE=..\..\..\IoVM\IoExceptionCatch.h
# End Source File
# Begin Source File

SOURCE=..\..\..\IoVM\IoFile.h
# End Source File
# Begin Source File

SOURCE=..\..\..\IoVM\IoJITFunction.h
# End Source File
# Begin Source File

SOURCE=..\..\..\IoVM\IoLinkedList.h
# End Source File
# Begin Source File

SOURCE=..\..\..\IoVM\IoList.h
# End Source File
# Begin Source File

SOURCE=..\..\..\IoVM\IoMap.h
# End Source File
# Begin Source File

SOURCE=..\..\..\IoVM\IoMark.h
# End Source File
# Begin Source File

SOURCE=..\..\..\IoVM\IoMessage.h
# End Source File
# Begin Source File

SOURCE=..\..\..\IoVM\IoMessage_parser.h
# End Source File
# Begin Source File

SOURCE=..\..\..\IoVM\IoNil.h
# End Source File
# Begin Source File

SOURCE=..\..\..\IoVM\IoNumber.h
# End Source File
# Begin Source File

SOURCE=..\..\..\IoVM\IoObject.h
# End Source File
# Begin Source File

SOURCE=..\..\..\IoVM\IoObject_actor.h
# End Source File
# Begin Source File

SOURCE=..\..\..\IoVM\IoState.h
# End Source File
# Begin Source File

SOURCE=..\..\..\IoVM\IoString.h
# End Source File
# Begin Source File

SOURCE=..\..\..\IoVM\IoTag.h
# End Source File
# Begin Source File

SOURCE=..\..\..\IoVM\IoValue.h
# End Source File
# Begin Source File

SOURCE=..\..\..\IoVM\IoValueGroup.h
# End Source File
# Begin Source File

SOURCE=..\..\..\IoVM\IoVersion.h
# End Source File
# Begin Source File

SOURCE=..\..\..\IoVM\IoVM.h
# End Source File
# Begin Source File

SOURCE=..\..\..\IoVM\IoWeakLink.h
# End Source File
# Begin Source File

SOURCE=..\..\..\IoVM\base\LinkedList.h
# End Source File
# Begin Source File

SOURCE=..\..\..\IoVM\base\List.h
# End Source File
# Begin Source File

SOURCE=..\..\..\IoVM\base\MainArgs.h
# End Source File
# Begin Source File

SOURCE=..\..\..\IoVM\base\Parser.h
# End Source File
# Begin Source File

SOURCE=..\..\..\IoVM\base\PHash.h
# End Source File
# Begin Source File

SOURCE=..\..\..\IoVM\base\PortableStrptime.h
# End Source File
# Begin Source File

SOURCE=..\..\..\IoVM\base\Scheduler.h
# End Source File
# Begin Source File

SOURCE=..\..\..\IoVM\base\SplayTree.h
# End Source File
# Begin Source File

SOURCE=..\..\..\IoVM\base\Stack.h
# End Source File
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;rgs;gif;jpg;jpeg;jpe"
# End Group
# End Target
# End Project
