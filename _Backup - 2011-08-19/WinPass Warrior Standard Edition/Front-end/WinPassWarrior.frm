VERSION 5.00
Object = "{831FDD16-0C5C-11D2-A9FC-0000F8754DA1}#2.0#0"; "mscomctl.ocx"
Begin VB.Form WinPassWarrior 
   BackColor       =   &H80000005&
   BorderStyle     =   1  'Fixed Single
   Caption         =   "WinPass Warrior Standard Edition"
   ClientHeight    =   8430
   ClientLeft      =   45
   ClientTop       =   435
   ClientWidth     =   8985
   Icon            =   "WinPassWarrior.frx":0000
   LinkTopic       =   "Form1"
   MaxButton       =   0   'False
   MinButton       =   0   'False
   ScaleHeight     =   8430
   ScaleWidth      =   8985
   StartUpPosition =   3  'Windows Default
   Begin VB.CommandButton cmdUseDomain 
      Caption         =   "Use Domain"
      BeginProperty Font 
         Name            =   "Arial"
         Size            =   9.75
         Charset         =   0
         Weight          =   400
         Underline       =   0   'False
         Italic          =   0   'False
         Strikethrough   =   0   'False
      EndProperty
      Height          =   255
      Left            =   4200
      TabIndex        =   2
      Top             =   1920
      Width           =   1215
   End
   Begin VB.TextBox txtPasswordLength 
      BeginProperty Font 
         Name            =   "Arial"
         Size            =   9.75
         Charset         =   0
         Weight          =   400
         Underline       =   0   'False
         Italic          =   0   'False
         Strikethrough   =   0   'False
      EndProperty
      Height          =   360
      Left            =   3120
      TabIndex        =   6
      Top             =   3120
      Width           =   495
   End
   Begin VB.OptionButton optSetRandomPassword 
      BackColor       =   &H80000005&
      Caption         =   "Set Random Password"
      BeginProperty Font 
         Name            =   "Arial"
         Size            =   9.75
         Charset         =   0
         Weight          =   400
         Underline       =   0   'False
         Italic          =   0   'False
         Strikethrough   =   0   'False
      EndProperty
      Height          =   375
      Left            =   3120
      TabIndex        =   5
      Top             =   2760
      Width           =   2295
   End
   Begin VB.TextBox txtPassword 
      BeginProperty Font 
         Name            =   "Arial"
         Size            =   9.75
         Charset         =   0
         Weight          =   400
         Underline       =   0   'False
         Italic          =   0   'False
         Strikethrough   =   0   'False
      EndProperty
      Height          =   360
      Left            =   360
      TabIndex        =   4
      Top             =   3120
      Width           =   2295
   End
   Begin VB.OptionButton optSetPassword 
      BackColor       =   &H80000005&
      Caption         =   "Set Manual Password"
      BeginProperty Font 
         Name            =   "Arial"
         Size            =   9.75
         Charset         =   0
         Weight          =   400
         Underline       =   0   'False
         Italic          =   0   'False
         Strikethrough   =   0   'False
      EndProperty
      Height          =   375
      Left            =   360
      TabIndex        =   3
      Top             =   2760
      Width           =   2295
   End
   Begin VB.TextBox txtGuestAccountName 
      BeginProperty Font 
         Name            =   "Arial"
         Size            =   9.75
         Charset         =   0
         Weight          =   400
         Underline       =   0   'False
         Italic          =   0   'False
         Strikethrough   =   0   'False
      EndProperty
      Height          =   360
      Left            =   3120
      TabIndex        =   11
      Top             =   3960
      Width           =   2295
   End
   Begin VB.TextBox txtAdminAccountName 
      BeginProperty Font 
         Name            =   "Arial"
         Size            =   9.75
         Charset         =   0
         Weight          =   400
         Underline       =   0   'False
         Italic          =   0   'False
         Strikethrough   =   0   'False
      EndProperty
      Height          =   360
      Left            =   360
      TabIndex        =   9
      Top             =   3960
      Width           =   2295
   End
   Begin VB.CheckBox chkDisableGuestAccount 
      BackColor       =   &H80000005&
      Caption         =   "Disable builtin Guest Account"
      BeginProperty Font 
         Name            =   "Arial"
         Size            =   9.75
         Charset         =   0
         Weight          =   400
         Underline       =   0   'False
         Italic          =   0   'False
         Strikethrough   =   0   'False
      EndProperty
      Height          =   255
      Left            =   360
      TabIndex        =   12
      Top             =   4440
      Value           =   1  'Checked
      Width           =   5055
   End
   Begin VB.TextBox txtStatus 
      BeginProperty Font 
         Name            =   "Courier New"
         Size            =   8.25
         Charset         =   0
         Weight          =   400
         Underline       =   0   'False
         Italic          =   0   'False
         Strikethrough   =   0   'False
      EndProperty
      Height          =   2415
      Left            =   360
      Locked          =   -1  'True
      MultiLine       =   -1  'True
      ScrollBars      =   3  'Both
      TabIndex        =   13
      Top             =   5400
      Width           =   8295
   End
   Begin VB.CommandButton cmdBrowse 
      Caption         =   "Browse"
      BeginProperty Font 
         Name            =   "Arial"
         Size            =   9.75
         Charset         =   0
         Weight          =   400
         Underline       =   0   'False
         Italic          =   0   'False
         Strikethrough   =   0   'False
      EndProperty
      Height          =   255
      Left            =   4200
      TabIndex        =   1
      Top             =   1680
      Width           =   1215
   End
   Begin VB.CommandButton cmdClear 
      Caption         =   "Clear"
      BeginProperty Font 
         Name            =   "Arial"
         Size            =   11.25
         Charset         =   0
         Weight          =   400
         Underline       =   0   'False
         Italic          =   0   'False
         Strikethrough   =   0   'False
      EndProperty
      Height          =   375
      Left            =   6600
      TabIndex        =   16
      Top             =   3240
      Width           =   1575
   End
   Begin VB.CommandButton cmdStartScan 
      Caption         =   "Start Scan"
      Default         =   -1  'True
      BeginProperty Font 
         Name            =   "Arial"
         Size            =   11.25
         Charset         =   0
         Weight          =   400
         Underline       =   0   'False
         Italic          =   0   'False
         Strikethrough   =   0   'False
      EndProperty
      Height          =   375
      Left            =   6600
      TabIndex        =   15
      Top             =   2760
      Width           =   1575
   End
   Begin VB.CheckBox chkRenameGuestAccount 
      BackColor       =   &H80000005&
      Caption         =   "Rename Guest"
      BeginProperty Font 
         Name            =   "Arial"
         Size            =   9.75
         Charset         =   0
         Weight          =   400
         Underline       =   0   'False
         Italic          =   0   'False
         Strikethrough   =   0   'False
      EndProperty
      Height          =   255
      Left            =   3120
      TabIndex        =   10
      Top             =   3600
      Value           =   1  'Checked
      Width           =   2295
   End
   Begin VB.CheckBox chkRenameAdminAccount 
      BackColor       =   &H80000005&
      Caption         =   "Rename Administrator"
      BeginProperty Font 
         Name            =   "Arial"
         Size            =   9.75
         Charset         =   0
         Weight          =   400
         Underline       =   0   'False
         Italic          =   0   'False
         Strikethrough   =   0   'False
      EndProperty
      Height          =   255
      Left            =   360
      TabIndex        =   8
      Top             =   3600
      Value           =   1  'Checked
      Width           =   2295
   End
   Begin VB.TextBox txtInput 
      BeginProperty Font 
         Name            =   "Arial"
         Size            =   9.75
         Charset         =   0
         Weight          =   400
         Underline       =   0   'False
         Italic          =   0   'False
         Strikethrough   =   0   'False
      EndProperty
      Height          =   360
      Left            =   360
      TabIndex        =   0
      Top             =   1725
      Width           =   3615
   End
   Begin MSComctlLib.ProgressBar pbProgress 
      Height          =   255
      Left            =   360
      TabIndex        =   14
      Top             =   7920
      Width           =   8295
      _ExtentX        =   14631
      _ExtentY        =   450
      _Version        =   393216
      Appearance      =   1
   End
   Begin VB.Label lblCharacters 
      BackColor       =   &H80000005&
      Caption         =   "characters"
      BeginProperty Font 
         Name            =   "Arial"
         Size            =   9.75
         Charset         =   0
         Weight          =   400
         Underline       =   0   'False
         Italic          =   -1  'True
         Strikethrough   =   0   'False
      EndProperty
      Height          =   255
      Left            =   3720
      TabIndex        =   7
      Top             =   3240
      Width           =   1695
   End
   Begin VB.Image Image1 
      Height          =   8430
      Left            =   0
      Picture         =   "WinPassWarrior.frx":2372
      Top             =   0
      Width           =   9000
   End
End
Attribute VB_Name = "WinPassWarrior"
Attribute VB_GlobalNameSpace = False
Attribute VB_Creatable = False
Attribute VB_PredeclaredId = True
Attribute VB_Exposed = False
Option Explicit

Private Const BIF_BROWSEINCLUDEFILES As Long = &H4000
Private Const MAX_PATH As Long = 512
Private Const STARTF_USESHOWWINDOW As Long = &H1
Private Const STARTF_USESTDHANDLES As Long = &H100
Private Const SW_HIDE As Integer = 0

Private Type BROWSEINFO
    hWndOwner      As Long
    pIDLRoot       As Long
    pszDisplayName As Long
    lpszTitle      As Long
    ulFlags        As Long
    lpfnCallback   As Long
    lParam         As Long
    iImage         As Long
End Type

Private Type SECURITY_ATTRIBUTES
    nLength              As Long
    lpSecurityDescriptor As Long
    bInheritHandle       As Long
End Type

Private Type STARTUPINFO
    cb              As Long
    lpReserved      As String
    lpDesktop       As String
    lpTitle         As String
    dwX             As Long
    dwY             As Long
    dwXSize         As Long
    dwYSize         As Long
    dwXCountChars   As Long
    dwYCountChars   As Long
    dwFillAttribute As Long
    dwFlags         As Long
    wShowWindow     As Integer
    cbReserved2     As Integer
    lpReserved2     As Long
    hStdInput       As Long
    hStdOutput      As Long
    hStdError       As Long
End Type

Private Type PROCESS_INFORMATION
    hProcess    As Long
    hThread     As Long
    dwProcessId As Long
    dwThreadId  As Long
End Type

Private Declare Function SHBrowseForFolder _
Lib "shell32" _
(lpbi As BROWSEINFO) _
As Long

Private Declare Function SHGetPathFromIDList _
Lib "shell32" _
(ByVal pidList As Long, _
 ByVal lpBuffer As String) _
As Long

Private Declare Function CreatePipe _
Lib "kernel32" _
(phReadPipe As Long, _
 phWritePipe As Long, _
 lpPipeAttributes As Any, _
 ByVal nSize As Long) _
As Long

Private Declare Function CreateProcess _
Lib "kernel32" Alias "CreateProcessA" _
(ByVal lpApplicationName As String, _
 ByVal lpCommandLine As String, _
 lpProcessAttributes As Any, _
 lpThreadAttributes As Any, _
 ByVal bInheritHandles As Long, _
 ByVal dwCreationFlags As Long, _
 lpEnvironment As Any, _
 ByVal lpCurrentDriectory As String, _
 lpStartupInfo As STARTUPINFO, _
 lpProcessInformation As PROCESS_INFORMATION) _
As Long

Private Declare Function ReadFile _
Lib "kernel32" _
(ByVal hFile As Long, _
 lpBuffer As Any, _
 ByVal nNumberOfBytesToRead As Long, _
 lpNumberOfBytesRead As Long, _
 lpOverlapped As Any) _
As Long

Private Declare Function CloseHandle _
Lib "kernel32" _
(ByVal hObject As Long) _
As Long

Private Sub Form_Load()
    optSetRandomPassword.Value = True

    txtPasswordLength.Text = "15"

    optSetPassword.Value = False

    txtPassword.Enabled = False

    txtPassword.Text = "<new password>"

    chkRenameAdminAccount.Value = 1

    txtAdminAccountName.Text = "RenamedAdmin"

    chkRenameGuestAccount.Value = 1

    txtGuestAccountName.Text = "RenamedGuest"

    chkDisableGuestAccount.Value = 1
End Sub

Private Sub cmdBrowse_Click()
    Dim lpIDList As Long
    Dim szBuffer As String
    Dim bi As BROWSEINFO
    Dim lReturn As Long

    bi.ulFlags = BIF_BROWSEINCLUDEFILES

    lpIDList = SHBrowseForFolder(bi)

    If (lpIDList) Then
        szBuffer = Space$(MAX_PATH)

        lReturn = SHGetPathFromIDList(lpIDList, szBuffer)

        If (lReturn) Then
            szBuffer = Left$(szBuffer, InStr(szBuffer, vbNullChar) - 1)

            txtInput.Text = szBuffer
        End If
    End If
End Sub

Private Sub cmdUseDomain_Click()
    Dim objWshShell As Object
    Dim strUserDomain As String
    Dim objDomain As Object
    Dim objFSO As Object
    Dim objFile As Object
    Dim i As Integer
    Dim objComputer As Object

    Const FOR_WRITING = 2

    cmdStartScan.Enabled = False
    cmdClear.Enabled = False

    WinPassWarrior.MousePointer = vbHourglass

    Set objWshShell = CreateObject("WScript.Shell")

    strUserDomain = ""

    strUserDomain = objWshShell.ExpandEnvironmentStrings("%USERDOMAIN%")

    strUserDomain = InputBox("Please enter the name of your Windows domain.", "Windows Domain Name", strUserDomain)

    If strUserDomain <> "" Then
        Set objDomain = GetObject("WinNT://" & strUserDomain)

        objDomain.Filter = Array("Computer")

        Set objFSO = CreateObject("Scripting.FileSystemObject")
        Set objFile = objFSO.OpenTextFile(strUserDomain & "_Domain_Machines.txt", FOR_WRITING, True)

        i = 0

        For Each objComputer In objDomain
            objFile.Write objComputer.Name & vbCrLf

            i = i + 1
        Next

        objFile.Close

        If i = 0 Then
            objFSO.DeleteFile strUserDomain & "_Domain_Machines.txt", True

            MsgBox "Unable to retrieve any machines from Windows domain " & strUserDomain & ".", vbCritical + vbOKOnly, "Error!"
        Else
            txtInput.Text = strUserDomain & "_Domain_Machines.txt"
        End If
    Else
        MsgBox "You must enter a Windows domain name.", vbCritical + vbOKOnly, "Error!"
    End If

    cmdStartScan.Enabled = True
    cmdClear.Enabled = True

    WinPassWarrior.MousePointer = vbArrow
End Sub

Private Sub chkRenameAdminAccount_Click()
    If chkRenameAdminAccount.Value = 1 Then
        txtAdminAccountName.Enabled = True

        txtAdminAccountName.Text = "RenamedAdmin"
    Else
        txtAdminAccountName.Enabled = False
    End If
End Sub

Private Sub chkRenameGuestAccount_Click()
    If chkRenameGuestAccount.Value = 1 Then
        txtGuestAccountName.Enabled = True

        txtGuestAccountName.Text = "RenamedGuest"
    Else
        txtGuestAccountName.Enabled = False
    End If
End Sub

Private Sub optSetPassword_Click()
    txtPassword.Enabled = True

    optSetRandomPassword.Value = False

    txtPasswordLength.Enabled = False
End Sub

Private Sub optSetRandomPassword_Click()
    txtPasswordLength.Enabled = True

    optSetPassword.Value = False

    txtPassword.Enabled = False
End Sub

Private Sub cmdStartScan_Click()
    Dim strArgs As String
    Dim intTargets As Integer
    Dim objFSO As Object
    Dim objFile As Object
    Dim strReadLine As String
    Dim sa As SECURITY_ATTRIBUTES
    Dim hPipeRead As Long
    Dim hPipeWrite As Long
    Dim strCommand As String
    Dim si As STARTUPINFO
    Dim pi As PROCESS_INFORMATION
    Dim i As Integer
    Dim lReturn As Long
    Dim bOutput(1024) As Byte
    Dim lBytesRead As Long
    Dim strOutput As String

    Const FOR_READING = 1

    If Trim(txtInput.Text) = "" Then
        MsgBox "Please enter a valid target host.", vbCritical + vbOKOnly, "Error!"

        Exit Sub
    End If

    pbProgress.Value = 0

    cmdStartScan.Enabled = False
    cmdClear.Enabled = False

    WinPassWarrior.MousePointer = vbHourglass

    strArgs = ""

    If optSetPassword.Value = True Then
        strArgs = strArgs & "-p " & txtPassword.Text & " "
    End If

    If optSetRandomPassword.Value = True Then
        strArgs = strArgs & "-r -l " & txtPasswordLength.Text & " "
    End If

    If chkRenameAdminAccount.Value = 1 Then
        strArgs = strArgs & "-a " & txtAdminAccountName.Text & " "
    End If

    If chkRenameGuestAccount.Value = 1 Then
        strArgs = strArgs & "-g " & txtGuestAccountName.Text & " "
    End If

    If chkDisableGuestAccount.Value = 1 Then
        strArgs = strArgs & "-d "
    End If

    intTargets = 1

    Set objFSO = CreateObject("Scripting.FileSystemObject")

    If objFSO.FileExists(txtInput.Text) Then
        Set objFile = objFSO.OpenTextFile(txtInput.Text, FOR_READING)

        intTargets = 0

        While Not objFile.AtEndOfStream
            strReadLine = objFile.ReadLine

            intTargets = intTargets + 1
        Wend

        objFile.Close
    End If

    If objFSO.FileExists(App.Path & "\WinPassWarrior.exe") Then
        sa.nLength = Len(sa)
        sa.bInheritHandle = 1

        If CreatePipe(hPipeRead, hPipeWrite, sa, 0) Then
            strCommand = """" & App.Path & "\WinPassWarrior.exe"" " & strArgs & " """ & txtInput.Text & """"

            si.cb = Len(si)
            si.dwFlags = STARTF_USESHOWWINDOW Or STARTF_USESTDHANDLES
            si.wShowWindow = SW_HIDE
            si.hStdOutput = hPipeWrite
            si.hStdError = hPipeWrite

            If CreateProcess(vbNullString, strCommand, ByVal 0&, ByVal 0&, 1, 0&, ByVal 0&, App.Path, si, pi) Then
                Call CloseHandle(hPipeWrite)
                Call CloseHandle(pi.hThread)

                hPipeWrite = 0

                i = 0

                Do
                    DoEvents
                        lReturn = ReadFile(hPipeRead, bOutput(0), 1024, lBytesRead, ByVal 0&)

                        If lReturn = 0 Then
                            Exit Do
                        End If

                        strOutput = Left$(StrConv(bOutput(), vbUnicode), lBytesRead)

                        txtStatus.SelStart = Len(txtStatus.Text)

                        txtStatus.SelText = strOutput

                        If InStr(strOutput, "Spawning thread") Then
                            i = i + 1

                            pbProgress.Value = (i / intTargets) * 100
                        End If
                Loop

                Call CloseHandle(pi.hProcess)
            End If

            Call CloseHandle(hPipeRead)
            Call CloseHandle(hPipeWrite)
        End If
    Else
        MsgBox "Cannot find file """ & App.Path & "\WinPassWarrior.exe"".", vbCritical + vbOKOnly, "Error!"
    End If

    pbProgress.Value = 100

    cmdStartScan.Enabled = True
    cmdClear.Enabled = True

    WinPassWarrior.MousePointer = vbArrow
End Sub

Private Sub cmdClear_Click()
    txtInput.Text = ""
    txtStatus.Text = ""

    pbProgress.Value = 0
End Sub
