VERSION 5.00
Object = "{831FDD16-0C5C-11D2-A9FC-0000F8754DA1}#2.0#0"; "mscomctl.ocx"
Begin VB.Form WinPassSafeCheck 
   BackColor       =   &H80000005&
   BorderStyle     =   1  'Fixed Single
   Caption         =   "WinPass Warrior Standard Edition"
   ClientHeight    =   8430
   ClientLeft      =   45
   ClientTop       =   435
   ClientWidth     =   8985
   Icon            =   "WinPassSafeCheck.frx":0000
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
   Begin VB.CheckBox chkUseExistingCredentials 
      BackColor       =   &H80000005&
      Caption         =   "Use Existing Credentials"
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
      TabIndex        =   6
      Top             =   3600
      Width           =   3735
   End
   Begin VB.CommandButton cmdBrowse2 
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
      Height          =   375
      Left            =   4200
      TabIndex        =   5
      Top             =   3120
      Width           =   1215
   End
   Begin VB.TextBox txtDictionary 
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
      Width           =   3615
   End
   Begin VB.TextBox txtConnectPassword 
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
      TabIndex        =   10
      Top             =   4200
      Width           =   2295
   End
   Begin VB.TextBox txtConnectUsername 
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
      TabIndex        =   8
      Top             =   4200
      Width           =   2295
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
      TabIndex        =   11
      Top             =   5400
      Width           =   8295
   End
   Begin VB.CommandButton cmdBrowse1 
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
      TabIndex        =   14
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
      TabIndex        =   13
      Top             =   2760
      Width           =   1575
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
      TabIndex        =   12
      Top             =   7920
      Width           =   8295
      _ExtentX        =   14631
      _ExtentY        =   450
      _Version        =   393216
      Appearance      =   1
   End
   Begin VB.Label lblConnectPassword 
      BackColor       =   &H80000005&
      Caption         =   "Connection Password"
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
      TabIndex        =   9
      Top             =   3960
      Width           =   2295
   End
   Begin VB.Label lblConnectUsername 
      BackColor       =   &H80000005&
      Caption         =   "Connection Username"
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
      TabIndex        =   7
      Top             =   3960
      Width           =   2295
   End
   Begin VB.Label lblDictionary 
      BackColor       =   &H80000005&
      Caption         =   "Dictionary File"
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
      TabIndex        =   3
      Top             =   2880
      Width           =   2295
   End
   Begin VB.Image Image1 
      Height          =   8430
      Left            =   0
      Picture         =   "WinPassSafeCheck.frx":2372
      Top             =   0
      Width           =   9000
   End
End
Attribute VB_Name = "WinPassSafeCheck"
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
    txtDictionary.Text = "dict.txt"

    chkUseExistingCredentials.Value = 1

    txtConnectUsername.Enabled = False

    txtConnectUsername.Text = "<username>"

    txtConnectPassword.Enabled = False

    txtConnectPassword.Text = "<password>"
End Sub

Private Sub cmdBrowse1_Click()
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

    WinPassSafeCheck.MousePointer = vbHourglass

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

    WinPassSafeCheck.MousePointer = vbArrow
End Sub

Private Sub cmdBrowse2_Click()
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

            txtDictionary.Text = szBuffer
        End If
    End If
End Sub

Private Sub chkUseExistingCredentials_Click()
    If chkUseExistingCredentials.Value = 1 Then
        txtConnectUsername.Enabled = False

        txtConnectPassword.Enabled = False
    Else
        txtConnectUsername.Enabled = True

        txtConnectUsername.Text = "<username>"

        txtConnectPassword.Enabled = True

        txtConnectPassword.Text = "<password>"
    End If
End Sub

Private Sub cmdStartScan_Click()
    Dim objFSO As Object
    Dim strArgs As String
    Dim intTargets As Integer
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

    Set objFSO = CreateObject("Scripting.FileSystemObject")

    If Not objFSO.FileExists(txtDictionary.Text) Then
        MsgBox "Cannot open dictionary file " & txtDictionary.Text & ".", vbCritical + vbOKOnly, "Error!"

        Exit Sub
    End If

    pbProgress.Value = 0

    cmdStartScan.Enabled = False
    cmdClear.Enabled = False

    WinPassSafeCheck.MousePointer = vbHourglass

    strArgs = ""

    strArgs = strArgs & "-d " & txtDictionary.Text & " "

    If chkUseExistingCredentials.Value = 1 Then
        strArgs = strArgs & "-u + -p + "
    Else
        strArgs = strArgs & "-u " & txtConnectUsername.Text & " -p " & txtConnectPassword.Text & " "
    End If

    intTargets = 1

    If objFSO.FileExists(txtInput.Text) Then
        Set objFile = objFSO.OpenTextFile(txtInput.Text, FOR_READING)

        intTargets = 0

        While Not objFile.AtEndOfStream
            strReadLine = objFile.ReadLine

            intTargets = intTargets + 1
        Wend

        objFile.Close
    End If

    If objFSO.FileExists(App.Path & "\WinPassSafeCheck.exe") Then
        sa.nLength = Len(sa)
        sa.bInheritHandle = 1

        If CreatePipe(hPipeRead, hPipeWrite, sa, 0) Then
            strCommand = """" & App.Path & "\WinPassSafeCheck.exe"" " & strArgs & " """ & txtInput.Text & """"

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
        MsgBox "Cannot find file """ & App.Path & "\WinPassSafeCheck.exe"".", vbCritical + vbOKOnly, "Error!"
    End If

    pbProgress.Value = 100

    cmdStartScan.Enabled = True
    cmdClear.Enabled = True

    WinPassSafeCheck.MousePointer = vbArrow
End Sub

Private Sub cmdClear_Click()
    txtInput.Text = ""
    txtStatus.Text = ""

    pbProgress.Value = 0
End Sub
