Public Class WinScanX
    Private Sub frmWinScanX_Load(ByVal sender As System.Object, ByVal e As System.EventArgs) Handles MyBase.Load

    End Sub

    Private Sub btnBrowse_Click(ByVal sender As System.Object, ByVal e As System.EventArgs) Handles btnBrowse.Click
        OpenFileDialog1.FileName = ""
        OpenFileDialog1.Filter = "All files (*.*)|*.*"
        OpenFileDialog1.InitialDirectory = Application.StartupPath

        If (OpenFileDialog1.ShowDialog() = DialogResult.OK) Then
            txtTargetInput.Text = OpenFileDialog1.FileName
        End If
    End Sub

    Private Sub btnUseDomain_Click(ByVal sender As System.Object, ByVal e As System.EventArgs) Handles btnUseDomain.Click
        Dim strUserDomain As String
        Dim objFS As FileStream
        Dim objSW As StreamWriter
        Dim objDomain As DirectoryEntry
        Dim i As Integer
        Dim objComputer As DirectoryEntry

        btnBrowse.Enabled = False
        btnUseDomain.Enabled = False
        btnStartScan.Enabled = False
        btnClear.Enabled = False
        btnViewReports.Enabled = False

        strUserDomain = Environment.UserDomainName

        strUserDomain = InputBox("Please enter the name of your Windows domain.", "Windows Domain Name", strUserDomain)

        If strUserDomain <> "" Then
            objFS = New FileStream(strUserDomain & "_Domain_Machines.txt", FileMode.Create)
            objSW = New StreamWriter(objFS)

            objDomain = New DirectoryEntry("WinNT://" & strUserDomain)

            objDomain.Children.SchemaFilter.Add("Computer")

            i = 0

            For Each objComputer In objDomain.Children
                objSW.WriteLine(objComputer.Name)

                i = i + 1
            Next

            objSW.Close()
            objFS.Close()

            If i = 0 Then
                If System.IO.File.Exists(strUserDomain & "_Domain_Machines.txt") Then
                    System.IO.File.Delete(strUserDomain & "_Domain_Machines.txt")
                End If

                MsgBox("Unable to retrieve any machines from the Windows domain """ & strUserDomain & """.", vbCritical + vbOKOnly, "Error!")
            Else
                txtTargetInput.Text = strUserDomain & "_Domain_Machines.txt"

                MsgBox("Windows domain machine list created successfully!", vbInformation + vbOKOnly, "Windows Domain Machine List")
            End If
        Else
            MsgBox("You must enter a Windows domain name.", vbCritical + vbOKOnly, "Error!")
        End If

        btnBrowse.Enabled = True
        btnUseDomain.Enabled = True
        btnStartScan.Enabled = True
        btnClear.Enabled = True
        btnViewReports.Enabled = True
    End Sub

    Private Sub optUseExistingCredentials_CheckedChanged(ByVal sender As System.Object, ByVal e As System.EventArgs) Handles optUseExistingCredentials.CheckedChanged
        txtUsername.Enabled = False
        txtPassword.Enabled = False
    End Sub

    Private Sub optUseAnonymousConnection_CheckedChanged(ByVal sender As System.Object, ByVal e As System.EventArgs) Handles optUseAnonymousConnection.CheckedChanged
        txtUsername.Enabled = False
        txtPassword.Enabled = False
    End Sub

    Private Sub optSpecifyUsernamePassword_CheckedChanged(ByVal sender As System.Object, ByVal e As System.EventArgs) Handles optSpecifyUsernamePassword.CheckedChanged
        txtUsername.Enabled = True
        txtPassword.Enabled = True
    End Sub

    Private Sub optGroup1_CheckedChanged(ByVal sender As System.Object, ByVal e As System.EventArgs) Handles optGroup1.CheckedChanged
        chkGetAccountPolicyInfo.Checked = True
        chkGetAuditPolicyInfo.Checked = False
        chkGetDisplayInfo.Checked = False
        chkGetDomainInfo.Checked = True
        chkGetLDAPInfo.Checked = False
        chkGetAdminLocalGlobalGroupInfo.Checked = False
        chkGetLocalGlobalGroupInfo.Checked = True
        chkGetInstalledPrograms.Checked = False
        chkGetInteractivelyLoggedOnUsers.Checked = False
        chkGetLoggedOnUsers.Checked = True
        chkGetPatchInfo.Checked = False
        chkGetRegistryInfo.Checked = False
        chkGetScheduledTaskInfo.Checked = False
        chkGetServerInfo.Checked = True
        chkGetServiceInfo.Checked = False
        chkGetShareInfo.Checked = True
        chkGetSharePermissions.Checked = False
        chkGetSNMPCommunityInfo.Checked = False
        chkGetUserInfo.Checked = True
        chkGetRAUserInfo.Checked = True
        chkGetUserRightsInfo.Checked = False
        chkGetWinVNCPasswords.Checked = False
        chkSaveRemoteRegistryHives.Checked = False
        chkGuessSNMPCommunityStrings.Checked = False
        chkGuessWindowsPasswords.Checked = False
        chkPingRemoteHost.Checked = True
    End Sub

    Private Sub optGroup2_CheckedChanged(ByVal sender As System.Object, ByVal e As System.EventArgs) Handles optGroup2.CheckedChanged
        chkGetAccountPolicyInfo.Checked = True
        chkGetAuditPolicyInfo.Checked = False
        chkGetDisplayInfo.Checked = False
        chkGetDomainInfo.Checked = True
        chkGetLDAPInfo.Checked = False
        chkGetAdminLocalGlobalGroupInfo.Checked = False
        chkGetLocalGlobalGroupInfo.Checked = True
        chkGetInstalledPrograms.Checked = True
        chkGetInteractivelyLoggedOnUsers.Checked = False
        chkGetLoggedOnUsers.Checked = True
        chkGetPatchInfo.Checked = False
        chkGetRegistryInfo.Checked = True
        chkGetScheduledTaskInfo.Checked = False
        chkGetServerInfo.Checked = True
        chkGetServiceInfo.Checked = False
        chkGetShareInfo.Checked = True
        chkGetSharePermissions.Checked = False
        chkGetSNMPCommunityInfo.Checked = True
        chkGetUserInfo.Checked = True
        chkGetRAUserInfo.Checked = False
        chkGetUserRightsInfo.Checked = False
        chkGetWinVNCPasswords.Checked = True
        chkSaveRemoteRegistryHives.Checked = False
        chkGuessSNMPCommunityStrings.Checked = False
        chkGuessWindowsPasswords.Checked = False
        chkPingRemoteHost.Checked = True
    End Sub

    Private Sub optGroup3_CheckedChanged(ByVal sender As System.Object, ByVal e As System.EventArgs) Handles optGroup3.CheckedChanged
        chkGetAccountPolicyInfo.Checked = True
        chkGetAuditPolicyInfo.Checked = True
        chkGetDisplayInfo.Checked = False
        chkGetDomainInfo.Checked = True
        chkGetLDAPInfo.Checked = False
        chkGetAdminLocalGlobalGroupInfo.Checked = False
        chkGetLocalGlobalGroupInfo.Checked = True
        chkGetInstalledPrograms.Checked = True
        chkGetInteractivelyLoggedOnUsers.Checked = False
        chkGetLoggedOnUsers.Checked = True
        chkGetPatchInfo.Checked = True
        chkGetRegistryInfo.Checked = True
        chkGetScheduledTaskInfo.Checked = True
        chkGetServerInfo.Checked = True
        chkGetServiceInfo.Checked = True
        chkGetShareInfo.Checked = True
        chkGetSharePermissions.Checked = True
        chkGetSNMPCommunityInfo.Checked = True
        chkGetUserInfo.Checked = True
        chkGetRAUserInfo.Checked = False
        chkGetUserRightsInfo.Checked = True
        chkGetWinVNCPasswords.Checked = True
        chkSaveRemoteRegistryHives.Checked = False
        chkGuessSNMPCommunityStrings.Checked = False
        chkGuessWindowsPasswords.Checked = False
        chkPingRemoteHost.Checked = True
    End Sub

    Private Sub btnStartScan_Click(ByVal sender As System.Object, ByVal e As System.EventArgs) Handles btnStartScan.Click
        Dim strUsername As String
        Dim strArgs As String
        Dim strPassword As String
        Dim intTargets As Integer
        Dim objFS As FileStream
        Dim objSW As StreamReader
        Dim strLine As String
        Dim i As Integer
        Dim objProcess As Process
        Dim objStdOut As StreamReader
        Dim objStdErr As StreamReader
        Dim strOutput As String

        btnBrowse.Enabled = False
        btnUseDomain.Enabled = False
        btnStartScan.Enabled = False
        btnClear.Enabled = False
        btnViewReports.Enabled = False

        If Trim(txtTargetInput.Text) <> "" Then
            If System.IO.File.Exists(Application.StartupPath & "\WinScanX.exe") Then
                Me.Cursor = Cursors.WaitCursor

                txtOutput.Text = ""

                ProgressBar1.Value = 0

                strUsername = ""
                strPassword = ""
                strArgs = ""

                If optUseExistingCredentials.Checked Then
                    strUsername = "+"
                    strPassword = "+"
                End If

                If optUseAnonymousConnection.Checked Then
                    strUsername = ""
                    strPassword = ""
                End If

                If optSpecifyUsernamePassword.Checked Then
                    strUsername = txtUsername.Text
                    strPassword = txtPassword.Text
                End If

                If chkGetAccountPolicyInfo.Checked Then
                    strArgs = strArgs & "a"
                End If

                If chkGetAuditPolicyInfo.Checked Then
                    strArgs = strArgs & "b"
                End If

                If chkGetDisplayInfo.Checked Then
                    strArgs = strArgs & "c"
                End If

                If chkGetDomainInfo.Checked Then
                    strArgs = strArgs & "d"
                End If

                If chkGetLDAPInfo.Checked Then
                    strArgs = strArgs & "e"
                End If

                If chkGetAdminLocalGlobalGroupInfo.Checked Then
                    strArgs = strArgs & "f"
                End If

                If chkGetLocalGlobalGroupInfo.Checked Then
                    strArgs = strArgs & "g"
                End If

                If chkGetInstalledPrograms.Checked Then
                    strArgs = strArgs & "p"
                End If

                If chkGetInteractivelyLoggedOnUsers.Checked Then
                    strArgs = strArgs & "k"
                End If

                If chkGetLoggedOnUsers.Checked Then
                    strArgs = strArgs & "l"
                End If

                If chkGetPatchInfo.Checked Then
                    strArgs = strArgs & "i"
                End If

                If chkGetRegistryInfo.Checked Then
                    strArgs = strArgs & "j"
                End If

                If chkGetScheduledTaskInfo.Checked Then
                    strArgs = strArgs & "m"
                End If

                If chkGetServerInfo.Checked Then
                    strArgs = strArgs & "n"
                End If

                If chkGetServiceInfo.Checked Then
                    strArgs = strArgs & "o"
                End If

                If chkGetShareInfo.Checked Then
                    strArgs = strArgs & "s"
                End If

                If chkGetSharePermissions.Checked Then
                    strArgs = strArgs & "t"
                End If

                If chkGetSNMPCommunityInfo.Checked Then
                    strArgs = strArgs & "q"
                End If

                If chkGetUserInfo.Checked Then
                    strArgs = strArgs & "u"
                End If

                If chkGetRAUserInfo.Checked Then
                    strArgs = strArgs & "r"
                End If

                If chkGetUserRightsInfo.Checked Then
                    strArgs = strArgs & "x"
                End If

                If chkGetWinVNCPasswords.Checked Then
                    strArgs = strArgs & "w"
                End If

                If chkSaveRemoteRegistryHives.Checked Then
                    strArgs = strArgs & "y"
                End If

                If chkGuessSNMPCommunityStrings.Checked Then
                    strArgs = strArgs & "S"
                End If

                If chkGuessWindowsPasswords.Checked Then
                    strArgs = strArgs & "W"
                End If

                If chkPingRemoteHost.Checked Then
                    strArgs = strArgs & "z"
                End If

                If strArgs = "" Then
                    If optGroup1.Checked Then
                        strArgs = "-1"
                    End If

                    If optGroup2.Checked Then
                        strArgs = "-2"
                    End If

                    If optGroup3.Checked Then
                        strArgs = "-3"
                    End If
                End If

                If System.IO.File.Exists(txtTargetInput.Text) Then
                    intTargets = 0

                    objFS = New FileStream(txtTargetInput.Text, FileMode.Open)
                    objSW = New StreamReader(objFS)

                    While Not objSW.EndOfStream
                        strLine = objSW.ReadLine

                        If Trim(strLine) <> "" Then
                            intTargets = intTargets + 1
                        End If
                    End While
                Else
                    ProgressBar1.Value = 50
                End If

                i = 0

                objProcess = New Process()

                objProcess.StartInfo.UseShellExecute = False
                objProcess.StartInfo.FileName = Application.StartupPath & "\WinScanX.exe"
                objProcess.StartInfo.Arguments = "-" & strArgs & " """ & txtTargetInput.Text & """ """ & strUsername & """ """ & strPassword & """"
                objProcess.StartInfo.CreateNoWindow = True
                objProcess.StartInfo.RedirectStandardOutput = True
                objProcess.StartInfo.RedirectStandardError = True

                objProcess.Start()

                objStdOut = objProcess.StandardOutput
                objStdErr = objProcess.StandardError

                While Not objStdOut.EndOfStream
                    strOutput = ""

                    strOutput = objStdOut.ReadLine

                    txtOutput.AppendText(strOutput & vbCrLf)

                    If InStr(UCase(strOutput), "SPAWNING") Then
                        i = i + 1

                        If (i / intTargets) * 100 < 95 Then
                            ProgressBar1.Value = (i / intTargets) * 100
                        Else
                            ProgressBar1.Value = 95
                        End If
                    End If
                End While

                objStdOut.Close()

                While Not objStdErr.EndOfStream
                    strOutput = ""

                    strOutput = objStdErr.ReadLine

                    txtOutput.AppendText(strOutput & vbCrLf)
                End While

                objStdErr.Close()

                txtOutput.AppendText(vbCrLf)
                txtOutput.AppendText("Finished!")

                ProgressBar1.Value = 100

                Me.Cursor = Cursors.Default
            Else
                MsgBox("Cannot find file """ & Application.StartupPath & "\WinScanX.exe"".", vbCritical + vbOKOnly, "Error!")
            End If
        Else
            MsgBox("Please enter a valid target host.", vbCritical + vbOKOnly, "Error!")
        End If

        btnBrowse.Enabled = True
        btnUseDomain.Enabled = True
        btnStartScan.Enabled = True
        btnClear.Enabled = True
        btnViewReports.Enabled = True
    End Sub

    Private Sub btnClear_Click(ByVal sender As System.Object, ByVal e As System.EventArgs) Handles btnClear.Click
        txtOutput.Text = ""
    End Sub

    Private Sub btnViewReports_Click(ByVal sender As System.Object, ByVal e As System.EventArgs) Handles btnViewReports.Click
        If System.IO.Directory.Exists(Application.StartupPath & "\Reports") Then
            System.Diagnostics.Process.Start(Application.StartupPath & "\Reports")
        End If
    End Sub

    Private Sub LinkLabel1_LinkClicked(ByVal sender As System.Object, ByVal e As System.Windows.Forms.LinkLabelLinkClickedEventArgs) Handles LinkLabel1.LinkClicked
        System.Diagnostics.Process.Start("http://www.windowsaudit.com/")
    End Sub
End Class