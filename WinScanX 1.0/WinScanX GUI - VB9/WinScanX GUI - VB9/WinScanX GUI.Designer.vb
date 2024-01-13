<Global.Microsoft.VisualBasic.CompilerServices.DesignerGenerated()> _
Partial Class WinScanX
    Inherits System.Windows.Forms.Form

    'Form overrides dispose to clean up the component list.
    <System.Diagnostics.DebuggerNonUserCode()> _
    Protected Overrides Sub Dispose(ByVal disposing As Boolean)
        Try
            If disposing AndAlso components IsNot Nothing Then
                components.Dispose()
            End If
        Finally
            MyBase.Dispose(disposing)
        End Try
    End Sub

    'Required by the Windows Form Designer
    Private components As System.ComponentModel.IContainer

    'NOTE: The following procedure is required by the Windows Form Designer
    'It can be modified using the Windows Form Designer.  
    'Do not modify it using the code editor.
    <System.Diagnostics.DebuggerStepThrough()> _
    Private Sub InitializeComponent()
        Dim resources As System.ComponentModel.ComponentResourceManager = New System.ComponentModel.ComponentResourceManager(GetType(WinScanX))
        Me.GroupBox1 = New System.Windows.Forms.GroupBox
        Me.txtTargetInput = New System.Windows.Forms.TextBox
        Me.btnUseDomain = New System.Windows.Forms.Button
        Me.btnBrowse = New System.Windows.Forms.Button
        Me.GroupBox2 = New System.Windows.Forms.GroupBox
        Me.GroupBox3 = New System.Windows.Forms.GroupBox
        Me.optUseExistingCredentials = New System.Windows.Forms.RadioButton
        Me.optUseAnonymousConnection = New System.Windows.Forms.RadioButton
        Me.optSpecifyUsernamePassword = New System.Windows.Forms.RadioButton
        Me.txtUsername = New System.Windows.Forms.TextBox
        Me.txtPassword = New System.Windows.Forms.TextBox
        Me.GroupBox4 = New System.Windows.Forms.GroupBox
        Me.optGroup1 = New System.Windows.Forms.RadioButton
        Me.optGroup3 = New System.Windows.Forms.RadioButton
        Me.optGroup2 = New System.Windows.Forms.RadioButton
        Me.chkSaveRemoteRegistryHives = New System.Windows.Forms.CheckBox
        Me.chkGetDomainInfo = New System.Windows.Forms.CheckBox
        Me.chkGuessWindowsPasswords = New System.Windows.Forms.CheckBox
        Me.chkGuessSNMPCommunityStrings = New System.Windows.Forms.CheckBox
        Me.chkPingRemoteHost = New System.Windows.Forms.CheckBox
        Me.chkGetWinVNCPasswords = New System.Windows.Forms.CheckBox
        Me.chkGetUserRightsInfo = New System.Windows.Forms.CheckBox
        Me.chkGetRAUserInfo = New System.Windows.Forms.CheckBox
        Me.chkGetUserInfo = New System.Windows.Forms.CheckBox
        Me.chkGetSNMPCommunityInfo = New System.Windows.Forms.CheckBox
        Me.chkGetSharePermissions = New System.Windows.Forms.CheckBox
        Me.chkGetShareInfo = New System.Windows.Forms.CheckBox
        Me.chkGetServiceInfo = New System.Windows.Forms.CheckBox
        Me.chkGetServerInfo = New System.Windows.Forms.CheckBox
        Me.chkGetScheduledTaskInfo = New System.Windows.Forms.CheckBox
        Me.chkGetRegistryInfo = New System.Windows.Forms.CheckBox
        Me.chkGetPatchInfo = New System.Windows.Forms.CheckBox
        Me.chkGetLoggedOnUsers = New System.Windows.Forms.CheckBox
        Me.chkGetInteractivelyLoggedOnUsers = New System.Windows.Forms.CheckBox
        Me.chkGetInstalledPrograms = New System.Windows.Forms.CheckBox
        Me.chkGetLocalGlobalGroupInfo = New System.Windows.Forms.CheckBox
        Me.chkGetAdminLocalGlobalGroupInfo = New System.Windows.Forms.CheckBox
        Me.chkGetLDAPInfo = New System.Windows.Forms.CheckBox
        Me.chkGetDisplayInfo = New System.Windows.Forms.CheckBox
        Me.chkGetAuditPolicyInfo = New System.Windows.Forms.CheckBox
        Me.chkGetAccountPolicyInfo = New System.Windows.Forms.CheckBox
        Me.GroupBox5 = New System.Windows.Forms.GroupBox
        Me.txtOutput = New System.Windows.Forms.TextBox
        Me.ProgressBar1 = New System.Windows.Forms.ProgressBar
        Me.btnStartScan = New System.Windows.Forms.Button
        Me.btnClear = New System.Windows.Forms.Button
        Me.LinkLabel1 = New System.Windows.Forms.LinkLabel
        Me.OpenFileDialog1 = New System.Windows.Forms.OpenFileDialog
        Me.btnViewReports = New System.Windows.Forms.Button
        Me.GroupBox1.SuspendLayout()
        Me.GroupBox2.SuspendLayout()
        Me.GroupBox3.SuspendLayout()
        Me.GroupBox4.SuspendLayout()
        Me.GroupBox5.SuspendLayout()
        Me.SuspendLayout()
        '
        'GroupBox1
        '
        Me.GroupBox1.Controls.Add(Me.txtTargetInput)
        Me.GroupBox1.Controls.Add(Me.btnUseDomain)
        Me.GroupBox1.Controls.Add(Me.btnBrowse)
        Me.GroupBox1.Location = New System.Drawing.Point(12, 12)
        Me.GroupBox1.Name = "GroupBox1"
        Me.GroupBox1.Size = New System.Drawing.Size(423, 55)
        Me.GroupBox1.TabIndex = 0
        Me.GroupBox1.TabStop = False
        Me.GroupBox1.Text = "Target Host(s)"
        '
        'txtTargetInput
        '
        Me.txtTargetInput.Font = New System.Drawing.Font("Arial", 9.0!, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, CType(0, Byte))
        Me.txtTargetInput.Location = New System.Drawing.Point(12, 20)
        Me.txtTargetInput.Name = "txtTargetInput"
        Me.txtTargetInput.Size = New System.Drawing.Size(233, 21)
        Me.txtTargetInput.TabIndex = 1
        '
        'btnUseDomain
        '
        Me.btnUseDomain.Font = New System.Drawing.Font("Arial", 8.25!, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, CType(0, Byte))
        Me.btnUseDomain.Location = New System.Drawing.Point(332, 18)
        Me.btnUseDomain.Name = "btnUseDomain"
        Me.btnUseDomain.Size = New System.Drawing.Size(75, 23)
        Me.btnUseDomain.TabIndex = 3
        Me.btnUseDomain.Text = "Use Domain"
        Me.btnUseDomain.UseVisualStyleBackColor = True
        '
        'btnBrowse
        '
        Me.btnBrowse.Font = New System.Drawing.Font("Arial", 8.25!, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, CType(0, Byte))
        Me.btnBrowse.Location = New System.Drawing.Point(251, 18)
        Me.btnBrowse.Name = "btnBrowse"
        Me.btnBrowse.Size = New System.Drawing.Size(75, 23)
        Me.btnBrowse.TabIndex = 2
        Me.btnBrowse.Text = "Browse"
        Me.btnBrowse.UseVisualStyleBackColor = True
        '
        'GroupBox2
        '
        Me.GroupBox2.Controls.Add(Me.GroupBox3)
        Me.GroupBox2.Controls.Add(Me.GroupBox4)
        Me.GroupBox2.Controls.Add(Me.chkSaveRemoteRegistryHives)
        Me.GroupBox2.Controls.Add(Me.chkGetDomainInfo)
        Me.GroupBox2.Controls.Add(Me.chkGuessWindowsPasswords)
        Me.GroupBox2.Controls.Add(Me.chkGuessSNMPCommunityStrings)
        Me.GroupBox2.Controls.Add(Me.chkPingRemoteHost)
        Me.GroupBox2.Controls.Add(Me.chkGetWinVNCPasswords)
        Me.GroupBox2.Controls.Add(Me.chkGetUserRightsInfo)
        Me.GroupBox2.Controls.Add(Me.chkGetRAUserInfo)
        Me.GroupBox2.Controls.Add(Me.chkGetUserInfo)
        Me.GroupBox2.Controls.Add(Me.chkGetSNMPCommunityInfo)
        Me.GroupBox2.Controls.Add(Me.chkGetSharePermissions)
        Me.GroupBox2.Controls.Add(Me.chkGetShareInfo)
        Me.GroupBox2.Controls.Add(Me.chkGetServiceInfo)
        Me.GroupBox2.Controls.Add(Me.chkGetServerInfo)
        Me.GroupBox2.Controls.Add(Me.chkGetScheduledTaskInfo)
        Me.GroupBox2.Controls.Add(Me.chkGetRegistryInfo)
        Me.GroupBox2.Controls.Add(Me.chkGetPatchInfo)
        Me.GroupBox2.Controls.Add(Me.chkGetLoggedOnUsers)
        Me.GroupBox2.Controls.Add(Me.chkGetInteractivelyLoggedOnUsers)
        Me.GroupBox2.Controls.Add(Me.chkGetInstalledPrograms)
        Me.GroupBox2.Controls.Add(Me.chkGetLocalGlobalGroupInfo)
        Me.GroupBox2.Controls.Add(Me.chkGetAdminLocalGlobalGroupInfo)
        Me.GroupBox2.Controls.Add(Me.chkGetLDAPInfo)
        Me.GroupBox2.Controls.Add(Me.chkGetDisplayInfo)
        Me.GroupBox2.Controls.Add(Me.chkGetAuditPolicyInfo)
        Me.GroupBox2.Controls.Add(Me.chkGetAccountPolicyInfo)
        Me.GroupBox2.Location = New System.Drawing.Point(12, 84)
        Me.GroupBox2.Name = "GroupBox2"
        Me.GroupBox2.Size = New System.Drawing.Size(753, 326)
        Me.GroupBox2.TabIndex = 4
        Me.GroupBox2.TabStop = False
        Me.GroupBox2.Text = "WinScanX Settings"
        '
        'GroupBox3
        '
        Me.GroupBox3.Controls.Add(Me.optUseExistingCredentials)
        Me.GroupBox3.Controls.Add(Me.optUseAnonymousConnection)
        Me.GroupBox3.Controls.Add(Me.optSpecifyUsernamePassword)
        Me.GroupBox3.Controls.Add(Me.txtUsername)
        Me.GroupBox3.Controls.Add(Me.txtPassword)
        Me.GroupBox3.Location = New System.Drawing.Point(12, 29)
        Me.GroupBox3.Name = "GroupBox3"
        Me.GroupBox3.Size = New System.Drawing.Size(206, 146)
        Me.GroupBox3.TabIndex = 5
        Me.GroupBox3.TabStop = False
        Me.GroupBox3.Text = "Connection Settings"
        '
        'optUseExistingCredentials
        '
        Me.optUseExistingCredentials.AutoSize = True
        Me.optUseExistingCredentials.Checked = True
        Me.optUseExistingCredentials.Font = New System.Drawing.Font("Arial", 8.25!, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, CType(0, Byte))
        Me.optUseExistingCredentials.Location = New System.Drawing.Point(8, 19)
        Me.optUseExistingCredentials.Name = "optUseExistingCredentials"
        Me.optUseExistingCredentials.Size = New System.Drawing.Size(141, 18)
        Me.optUseExistingCredentials.TabIndex = 6
        Me.optUseExistingCredentials.TabStop = True
        Me.optUseExistingCredentials.Text = "Use Existing Credentials"
        Me.optUseExistingCredentials.UseVisualStyleBackColor = True
        '
        'optUseAnonymousConnection
        '
        Me.optUseAnonymousConnection.AutoSize = True
        Me.optUseAnonymousConnection.Font = New System.Drawing.Font("Arial", 8.25!, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, CType(0, Byte))
        Me.optUseAnonymousConnection.Location = New System.Drawing.Point(8, 43)
        Me.optUseAnonymousConnection.Name = "optUseAnonymousConnection"
        Me.optUseAnonymousConnection.Size = New System.Drawing.Size(162, 18)
        Me.optUseAnonymousConnection.TabIndex = 7
        Me.optUseAnonymousConnection.Text = "Use Anonymous Connection"
        Me.optUseAnonymousConnection.UseVisualStyleBackColor = True
        '
        'optSpecifyUsernamePassword
        '
        Me.optSpecifyUsernamePassword.AutoSize = True
        Me.optSpecifyUsernamePassword.Font = New System.Drawing.Font("Arial", 8.25!, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, CType(0, Byte))
        Me.optSpecifyUsernamePassword.Location = New System.Drawing.Point(8, 67)
        Me.optSpecifyUsernamePassword.Name = "optSpecifyUsernamePassword"
        Me.optSpecifyUsernamePassword.Size = New System.Drawing.Size(177, 18)
        Me.optSpecifyUsernamePassword.TabIndex = 8
        Me.optSpecifyUsernamePassword.Text = "Specify Username && Password"
        Me.optSpecifyUsernamePassword.UseVisualStyleBackColor = True
        '
        'txtUsername
        '
        Me.txtUsername.Enabled = False
        Me.txtUsername.Font = New System.Drawing.Font("Arial", 8.25!, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, CType(0, Byte))
        Me.txtUsername.Location = New System.Drawing.Point(26, 91)
        Me.txtUsername.Name = "txtUsername"
        Me.txtUsername.Size = New System.Drawing.Size(164, 20)
        Me.txtUsername.TabIndex = 9
        Me.txtUsername.Text = "MyDomain\Administrator"
        '
        'txtPassword
        '
        Me.txtPassword.Enabled = False
        Me.txtPassword.Font = New System.Drawing.Font("Arial", 8.25!, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, CType(0, Byte))
        Me.txtPassword.Location = New System.Drawing.Point(26, 117)
        Me.txtPassword.Name = "txtPassword"
        Me.txtPassword.PasswordChar = Global.Microsoft.VisualBasic.ChrW(42)
        Me.txtPassword.Size = New System.Drawing.Size(164, 20)
        Me.txtPassword.TabIndex = 10
        '
        'GroupBox4
        '
        Me.GroupBox4.Controls.Add(Me.optGroup1)
        Me.GroupBox4.Controls.Add(Me.optGroup3)
        Me.GroupBox4.Controls.Add(Me.optGroup2)
        Me.GroupBox4.Location = New System.Drawing.Point(12, 190)
        Me.GroupBox4.Name = "GroupBox4"
        Me.GroupBox4.Size = New System.Drawing.Size(206, 100)
        Me.GroupBox4.TabIndex = 11
        Me.GroupBox4.TabStop = False
        Me.GroupBox4.Text = "Quick Setup"
        '
        'optGroup1
        '
        Me.optGroup1.AutoSize = True
        Me.optGroup1.Font = New System.Drawing.Font("Arial", 8.25!, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, CType(0, Byte))
        Me.optGroup1.Location = New System.Drawing.Point(6, 19)
        Me.optGroup1.Name = "optGroup1"
        Me.optGroup1.Size = New System.Drawing.Size(149, 18)
        Me.optGroup1.TabIndex = 12
        Me.optGroup1.Text = "Group 1 (No Permissions)"
        Me.optGroup1.UseVisualStyleBackColor = True
        '
        'optGroup3
        '
        Me.optGroup3.AutoSize = True
        Me.optGroup3.Font = New System.Drawing.Font("Arial", 8.25!, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, CType(0, Byte))
        Me.optGroup3.Location = New System.Drawing.Point(6, 67)
        Me.optGroup3.Name = "optGroup3"
        Me.optGroup3.Size = New System.Drawing.Size(191, 18)
        Me.optGroup3.TabIndex = 14
        Me.optGroup3.Text = "Group 3 (Admin level permissions)"
        Me.optGroup3.UseVisualStyleBackColor = True
        '
        'optGroup2
        '
        Me.optGroup2.AutoSize = True
        Me.optGroup2.Checked = True
        Me.optGroup2.Font = New System.Drawing.Font("Arial", 8.25!, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, CType(0, Byte))
        Me.optGroup2.Location = New System.Drawing.Point(6, 43)
        Me.optGroup2.Name = "optGroup2"
        Me.optGroup2.Size = New System.Drawing.Size(184, 18)
        Me.optGroup2.TabIndex = 13
        Me.optGroup2.TabStop = True
        Me.optGroup2.Text = "Group 2 (User level permissions)"
        Me.optGroup2.UseVisualStyleBackColor = True
        '
        'chkSaveRemoteRegistryHives
        '
        Me.chkSaveRemoteRegistryHives.AutoSize = True
        Me.chkSaveRemoteRegistryHives.Font = New System.Drawing.Font("Arial", 8.25!, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, CType(0, Byte))
        Me.chkSaveRemoteRegistryHives.Location = New System.Drawing.Point(537, 226)
        Me.chkSaveRemoteRegistryHives.Name = "chkSaveRemoteRegistryHives"
        Me.chkSaveRemoteRegistryHives.Size = New System.Drawing.Size(163, 18)
        Me.chkSaveRemoteRegistryHives.TabIndex = 37
        Me.chkSaveRemoteRegistryHives.Text = "Save Remote Registry Hives"
        Me.chkSaveRemoteRegistryHives.UseVisualStyleBackColor = True
        '
        'chkGetDomainInfo
        '
        Me.chkGetDomainInfo.AutoSize = True
        Me.chkGetDomainInfo.Checked = True
        Me.chkGetDomainInfo.CheckState = System.Windows.Forms.CheckState.Checked
        Me.chkGetDomainInfo.Font = New System.Drawing.Font("Arial", 8.25!, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, CType(0, Byte))
        Me.chkGetDomainInfo.Location = New System.Drawing.Point(249, 88)
        Me.chkGetDomainInfo.Name = "chkGetDomainInfo"
        Me.chkGetDomainInfo.Size = New System.Drawing.Size(137, 18)
        Me.chkGetDomainInfo.TabIndex = 18
        Me.chkGetDomainInfo.Text = "Get Domain Information"
        Me.chkGetDomainInfo.UseVisualStyleBackColor = True
        '
        'chkGuessWindowsPasswords
        '
        Me.chkGuessWindowsPasswords.AutoSize = True
        Me.chkGuessWindowsPasswords.Font = New System.Drawing.Font("Arial", 8.25!, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, CType(0, Byte))
        Me.chkGuessWindowsPasswords.Location = New System.Drawing.Point(537, 272)
        Me.chkGuessWindowsPasswords.Name = "chkGuessWindowsPasswords"
        Me.chkGuessWindowsPasswords.Size = New System.Drawing.Size(166, 18)
        Me.chkGuessWindowsPasswords.TabIndex = 39
        Me.chkGuessWindowsPasswords.Text = "Guess Windows Passwords"
        Me.chkGuessWindowsPasswords.UseVisualStyleBackColor = True
        '
        'chkGuessSNMPCommunityStrings
        '
        Me.chkGuessSNMPCommunityStrings.AutoSize = True
        Me.chkGuessSNMPCommunityStrings.Font = New System.Drawing.Font("Arial", 8.25!, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, CType(0, Byte))
        Me.chkGuessSNMPCommunityStrings.Location = New System.Drawing.Point(537, 249)
        Me.chkGuessSNMPCommunityStrings.Name = "chkGuessSNMPCommunityStrings"
        Me.chkGuessSNMPCommunityStrings.Size = New System.Drawing.Size(181, 18)
        Me.chkGuessSNMPCommunityStrings.TabIndex = 38
        Me.chkGuessSNMPCommunityStrings.Text = "Guess SNMP Community Strings"
        Me.chkGuessSNMPCommunityStrings.UseVisualStyleBackColor = True
        '
        'chkPingRemoteHost
        '
        Me.chkPingRemoteHost.AutoSize = True
        Me.chkPingRemoteHost.Checked = True
        Me.chkPingRemoteHost.CheckState = System.Windows.Forms.CheckState.Checked
        Me.chkPingRemoteHost.Font = New System.Drawing.Font("Arial", 8.25!, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, CType(0, Byte))
        Me.chkPingRemoteHost.Location = New System.Drawing.Point(537, 295)
        Me.chkPingRemoteHost.Name = "chkPingRemoteHost"
        Me.chkPingRemoteHost.Size = New System.Drawing.Size(194, 18)
        Me.chkPingRemoteHost.TabIndex = 40
        Me.chkPingRemoteHost.Text = "Ping Remote Host Before Scanning"
        Me.chkPingRemoteHost.UseVisualStyleBackColor = True
        '
        'chkGetWinVNCPasswords
        '
        Me.chkGetWinVNCPasswords.AutoSize = True
        Me.chkGetWinVNCPasswords.Checked = True
        Me.chkGetWinVNCPasswords.CheckState = System.Windows.Forms.CheckState.Checked
        Me.chkGetWinVNCPasswords.Font = New System.Drawing.Font("Arial", 8.25!, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, CType(0, Byte))
        Me.chkGetWinVNCPasswords.Location = New System.Drawing.Point(537, 203)
        Me.chkGetWinVNCPasswords.Name = "chkGetWinVNCPasswords"
        Me.chkGetWinVNCPasswords.Size = New System.Drawing.Size(210, 18)
        Me.chkGetWinVNCPasswords.TabIndex = 36
        Me.chkGetWinVNCPasswords.Text = "Get WinVNC3 && WinVNC4 Passwords"
        Me.chkGetWinVNCPasswords.UseVisualStyleBackColor = True
        '
        'chkGetUserRightsInfo
        '
        Me.chkGetUserRightsInfo.AutoSize = True
        Me.chkGetUserRightsInfo.Font = New System.Drawing.Font("Arial", 8.25!, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, CType(0, Byte))
        Me.chkGetUserRightsInfo.Location = New System.Drawing.Point(537, 180)
        Me.chkGetUserRightsInfo.Name = "chkGetUserRightsInfo"
        Me.chkGetUserRightsInfo.Size = New System.Drawing.Size(158, 18)
        Me.chkGetUserRightsInfo.TabIndex = 35
        Me.chkGetUserRightsInfo.Text = "Get User Rights Information"
        Me.chkGetUserRightsInfo.UseVisualStyleBackColor = True
        '
        'chkGetRAUserInfo
        '
        Me.chkGetRAUserInfo.AutoSize = True
        Me.chkGetRAUserInfo.Font = New System.Drawing.Font("Arial", 8.25!, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, CType(0, Byte))
        Me.chkGetRAUserInfo.Location = New System.Drawing.Point(537, 157)
        Me.chkGetRAUserInfo.Name = "chkGetRAUserInfo"
        Me.chkGetRAUserInfo.Size = New System.Drawing.Size(200, 18)
        Me.chkGetRAUserInfo.TabIndex = 34
        Me.chkGetRAUserInfo.Text = "Get User Information via RA Bypass"
        Me.chkGetRAUserInfo.UseVisualStyleBackColor = True
        '
        'chkGetUserInfo
        '
        Me.chkGetUserInfo.AutoSize = True
        Me.chkGetUserInfo.Checked = True
        Me.chkGetUserInfo.CheckState = System.Windows.Forms.CheckState.Checked
        Me.chkGetUserInfo.Font = New System.Drawing.Font("Arial", 8.25!, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, CType(0, Byte))
        Me.chkGetUserInfo.Location = New System.Drawing.Point(537, 134)
        Me.chkGetUserInfo.Name = "chkGetUserInfo"
        Me.chkGetUserInfo.Size = New System.Drawing.Size(125, 18)
        Me.chkGetUserInfo.TabIndex = 33
        Me.chkGetUserInfo.Text = "Get User Information"
        Me.chkGetUserInfo.UseVisualStyleBackColor = True
        '
        'chkGetSNMPCommunityInfo
        '
        Me.chkGetSNMPCommunityInfo.AutoSize = True
        Me.chkGetSNMPCommunityInfo.Checked = True
        Me.chkGetSNMPCommunityInfo.CheckState = System.Windows.Forms.CheckState.Checked
        Me.chkGetSNMPCommunityInfo.Font = New System.Drawing.Font("Arial", 8.25!, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, CType(0, Byte))
        Me.chkGetSNMPCommunityInfo.Location = New System.Drawing.Point(537, 111)
        Me.chkGetSNMPCommunityInfo.Name = "chkGetSNMPCommunityInfo"
        Me.chkGetSNMPCommunityInfo.Size = New System.Drawing.Size(185, 18)
        Me.chkGetSNMPCommunityInfo.TabIndex = 32
        Me.chkGetSNMPCommunityInfo.Text = "Get SNMP Community Information"
        Me.chkGetSNMPCommunityInfo.UseVisualStyleBackColor = True
        '
        'chkGetSharePermissions
        '
        Me.chkGetSharePermissions.AutoSize = True
        Me.chkGetSharePermissions.Font = New System.Drawing.Font("Arial", 8.25!, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, CType(0, Byte))
        Me.chkGetSharePermissions.Location = New System.Drawing.Point(537, 88)
        Me.chkGetSharePermissions.Name = "chkGetSharePermissions"
        Me.chkGetSharePermissions.Size = New System.Drawing.Size(136, 18)
        Me.chkGetSharePermissions.TabIndex = 31
        Me.chkGetSharePermissions.Text = "Get Share Permissions"
        Me.chkGetSharePermissions.UseVisualStyleBackColor = True
        '
        'chkGetShareInfo
        '
        Me.chkGetShareInfo.AutoSize = True
        Me.chkGetShareInfo.Checked = True
        Me.chkGetShareInfo.CheckState = System.Windows.Forms.CheckState.Checked
        Me.chkGetShareInfo.Font = New System.Drawing.Font("Arial", 8.25!, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, CType(0, Byte))
        Me.chkGetShareInfo.Location = New System.Drawing.Point(537, 65)
        Me.chkGetShareInfo.Name = "chkGetShareInfo"
        Me.chkGetShareInfo.Size = New System.Drawing.Size(131, 18)
        Me.chkGetShareInfo.TabIndex = 30
        Me.chkGetShareInfo.Text = "Get Share Information"
        Me.chkGetShareInfo.UseVisualStyleBackColor = True
        '
        'chkGetServiceInfo
        '
        Me.chkGetServiceInfo.AutoSize = True
        Me.chkGetServiceInfo.Font = New System.Drawing.Font("Arial", 8.25!, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, CType(0, Byte))
        Me.chkGetServiceInfo.Location = New System.Drawing.Point(537, 42)
        Me.chkGetServiceInfo.Name = "chkGetServiceInfo"
        Me.chkGetServiceInfo.Size = New System.Drawing.Size(139, 18)
        Me.chkGetServiceInfo.TabIndex = 29
        Me.chkGetServiceInfo.Text = "Get Service Information"
        Me.chkGetServiceInfo.UseVisualStyleBackColor = True
        '
        'chkGetServerInfo
        '
        Me.chkGetServerInfo.AutoSize = True
        Me.chkGetServerInfo.Checked = True
        Me.chkGetServerInfo.CheckState = System.Windows.Forms.CheckState.Checked
        Me.chkGetServerInfo.Font = New System.Drawing.Font("Arial", 8.25!, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, CType(0, Byte))
        Me.chkGetServerInfo.Location = New System.Drawing.Point(537, 19)
        Me.chkGetServerInfo.Name = "chkGetServerInfo"
        Me.chkGetServerInfo.Size = New System.Drawing.Size(135, 18)
        Me.chkGetServerInfo.TabIndex = 28
        Me.chkGetServerInfo.Text = "Get Server Information"
        Me.chkGetServerInfo.UseVisualStyleBackColor = True
        '
        'chkGetScheduledTaskInfo
        '
        Me.chkGetScheduledTaskInfo.AutoSize = True
        Me.chkGetScheduledTaskInfo.Font = New System.Drawing.Font("Arial", 8.25!, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, CType(0, Byte))
        Me.chkGetScheduledTaskInfo.Location = New System.Drawing.Point(249, 295)
        Me.chkGetScheduledTaskInfo.Name = "chkGetScheduledTaskInfo"
        Me.chkGetScheduledTaskInfo.Size = New System.Drawing.Size(179, 18)
        Me.chkGetScheduledTaskInfo.TabIndex = 27
        Me.chkGetScheduledTaskInfo.Text = "Get Scheduled Task Information"
        Me.chkGetScheduledTaskInfo.UseVisualStyleBackColor = True
        '
        'chkGetRegistryInfo
        '
        Me.chkGetRegistryInfo.AutoSize = True
        Me.chkGetRegistryInfo.Checked = True
        Me.chkGetRegistryInfo.CheckState = System.Windows.Forms.CheckState.Checked
        Me.chkGetRegistryInfo.Font = New System.Drawing.Font("Arial", 8.25!, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, CType(0, Byte))
        Me.chkGetRegistryInfo.Location = New System.Drawing.Point(249, 272)
        Me.chkGetRegistryInfo.Name = "chkGetRegistryInfo"
        Me.chkGetRegistryInfo.Size = New System.Drawing.Size(142, 18)
        Me.chkGetRegistryInfo.TabIndex = 26
        Me.chkGetRegistryInfo.Text = "Get Registry Information"
        Me.chkGetRegistryInfo.UseVisualStyleBackColor = True
        '
        'chkGetPatchInfo
        '
        Me.chkGetPatchInfo.AutoSize = True
        Me.chkGetPatchInfo.Font = New System.Drawing.Font("Arial", 8.25!, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, CType(0, Byte))
        Me.chkGetPatchInfo.Location = New System.Drawing.Point(249, 249)
        Me.chkGetPatchInfo.Name = "chkGetPatchInfo"
        Me.chkGetPatchInfo.Size = New System.Drawing.Size(129, 18)
        Me.chkGetPatchInfo.TabIndex = 25
        Me.chkGetPatchInfo.Text = "Get Patch Information"
        Me.chkGetPatchInfo.UseVisualStyleBackColor = True
        '
        'chkGetLoggedOnUsers
        '
        Me.chkGetLoggedOnUsers.AutoSize = True
        Me.chkGetLoggedOnUsers.Checked = True
        Me.chkGetLoggedOnUsers.CheckState = System.Windows.Forms.CheckState.Checked
        Me.chkGetLoggedOnUsers.Font = New System.Drawing.Font("Arial", 8.25!, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, CType(0, Byte))
        Me.chkGetLoggedOnUsers.Location = New System.Drawing.Point(249, 226)
        Me.chkGetLoggedOnUsers.Name = "chkGetLoggedOnUsers"
        Me.chkGetLoggedOnUsers.Size = New System.Drawing.Size(131, 18)
        Me.chkGetLoggedOnUsers.TabIndex = 24
        Me.chkGetLoggedOnUsers.Text = "Get Logged On Users"
        Me.chkGetLoggedOnUsers.UseVisualStyleBackColor = True
        '
        'chkGetInteractivelyLoggedOnUsers
        '
        Me.chkGetInteractivelyLoggedOnUsers.AutoSize = True
        Me.chkGetInteractivelyLoggedOnUsers.Font = New System.Drawing.Font("Arial", 8.25!, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, CType(0, Byte))
        Me.chkGetInteractivelyLoggedOnUsers.Location = New System.Drawing.Point(249, 203)
        Me.chkGetInteractivelyLoggedOnUsers.Name = "chkGetInteractivelyLoggedOnUsers"
        Me.chkGetInteractivelyLoggedOnUsers.Size = New System.Drawing.Size(192, 18)
        Me.chkGetInteractivelyLoggedOnUsers.TabIndex = 23
        Me.chkGetInteractivelyLoggedOnUsers.Text = "Get Interactively Logged On Users"
        Me.chkGetInteractivelyLoggedOnUsers.UseVisualStyleBackColor = True
        '
        'chkGetInstalledPrograms
        '
        Me.chkGetInstalledPrograms.AutoSize = True
        Me.chkGetInstalledPrograms.Checked = True
        Me.chkGetInstalledPrograms.CheckState = System.Windows.Forms.CheckState.Checked
        Me.chkGetInstalledPrograms.Font = New System.Drawing.Font("Arial", 8.25!, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, CType(0, Byte))
        Me.chkGetInstalledPrograms.Location = New System.Drawing.Point(249, 180)
        Me.chkGetInstalledPrograms.Name = "chkGetInstalledPrograms"
        Me.chkGetInstalledPrograms.Size = New System.Drawing.Size(134, 18)
        Me.chkGetInstalledPrograms.TabIndex = 22
        Me.chkGetInstalledPrograms.Text = "Get Installed Programs"
        Me.chkGetInstalledPrograms.UseVisualStyleBackColor = True
        '
        'chkGetLocalGlobalGroupInfo
        '
        Me.chkGetLocalGlobalGroupInfo.AutoSize = True
        Me.chkGetLocalGlobalGroupInfo.Checked = True
        Me.chkGetLocalGlobalGroupInfo.CheckState = System.Windows.Forms.CheckState.Checked
        Me.chkGetLocalGlobalGroupInfo.Font = New System.Drawing.Font("Arial", 8.25!, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, CType(0, Byte))
        Me.chkGetLocalGlobalGroupInfo.Location = New System.Drawing.Point(249, 157)
        Me.chkGetLocalGlobalGroupInfo.Name = "chkGetLocalGlobalGroupInfo"
        Me.chkGetLocalGlobalGroupInfo.Size = New System.Drawing.Size(204, 18)
        Me.chkGetLocalGlobalGroupInfo.TabIndex = 21
        Me.chkGetLocalGlobalGroupInfo.Text = "Get Local && Global Group Information"
        Me.chkGetLocalGlobalGroupInfo.UseVisualStyleBackColor = True
        '
        'chkGetAdminLocalGlobalGroupInfo
        '
        Me.chkGetAdminLocalGlobalGroupInfo.AutoSize = True
        Me.chkGetAdminLocalGlobalGroupInfo.Font = New System.Drawing.Font("Arial", 8.25!, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, CType(0, Byte))
        Me.chkGetAdminLocalGlobalGroupInfo.Location = New System.Drawing.Point(249, 134)
        Me.chkGetAdminLocalGlobalGroupInfo.Name = "chkGetAdminLocalGlobalGroupInfo"
        Me.chkGetAdminLocalGlobalGroupInfo.Size = New System.Drawing.Size(275, 18)
        Me.chkGetAdminLocalGlobalGroupInfo.TabIndex = 20
        Me.chkGetAdminLocalGlobalGroupInfo.Text = "Get Administrative Local && Global Group Information"
        Me.chkGetAdminLocalGlobalGroupInfo.UseVisualStyleBackColor = True
        '
        'chkGetLDAPInfo
        '
        Me.chkGetLDAPInfo.AutoSize = True
        Me.chkGetLDAPInfo.Font = New System.Drawing.Font("Arial", 8.25!, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, CType(0, Byte))
        Me.chkGetLDAPInfo.Location = New System.Drawing.Point(249, 111)
        Me.chkGetLDAPInfo.Name = "chkGetLDAPInfo"
        Me.chkGetLDAPInfo.Size = New System.Drawing.Size(129, 18)
        Me.chkGetLDAPInfo.TabIndex = 19
        Me.chkGetLDAPInfo.Text = "Get LDAP Information"
        Me.chkGetLDAPInfo.UseVisualStyleBackColor = True
        '
        'chkGetDisplayInfo
        '
        Me.chkGetDisplayInfo.AutoSize = True
        Me.chkGetDisplayInfo.Font = New System.Drawing.Font("Arial", 8.25!, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, CType(0, Byte))
        Me.chkGetDisplayInfo.Location = New System.Drawing.Point(249, 65)
        Me.chkGetDisplayInfo.Name = "chkGetDisplayInfo"
        Me.chkGetDisplayInfo.Size = New System.Drawing.Size(137, 18)
        Me.chkGetDisplayInfo.TabIndex = 17
        Me.chkGetDisplayInfo.Text = "Get Display Information"
        Me.chkGetDisplayInfo.UseVisualStyleBackColor = True
        '
        'chkGetAuditPolicyInfo
        '
        Me.chkGetAuditPolicyInfo.AutoSize = True
        Me.chkGetAuditPolicyInfo.Font = New System.Drawing.Font("Arial", 8.25!, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, CType(0, Byte))
        Me.chkGetAuditPolicyInfo.Location = New System.Drawing.Point(249, 42)
        Me.chkGetAuditPolicyInfo.Name = "chkGetAuditPolicyInfo"
        Me.chkGetAuditPolicyInfo.Size = New System.Drawing.Size(158, 18)
        Me.chkGetAuditPolicyInfo.TabIndex = 16
        Me.chkGetAuditPolicyInfo.Text = "Get Audit Policy Information"
        Me.chkGetAuditPolicyInfo.UseVisualStyleBackColor = True
        '
        'chkGetAccountPolicyInfo
        '
        Me.chkGetAccountPolicyInfo.AutoSize = True
        Me.chkGetAccountPolicyInfo.Checked = True
        Me.chkGetAccountPolicyInfo.CheckState = System.Windows.Forms.CheckState.Checked
        Me.chkGetAccountPolicyInfo.Font = New System.Drawing.Font("Arial", 8.25!, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, CType(0, Byte))
        Me.chkGetAccountPolicyInfo.Location = New System.Drawing.Point(249, 19)
        Me.chkGetAccountPolicyInfo.Name = "chkGetAccountPolicyInfo"
        Me.chkGetAccountPolicyInfo.Size = New System.Drawing.Size(174, 18)
        Me.chkGetAccountPolicyInfo.TabIndex = 15
        Me.chkGetAccountPolicyInfo.Text = "Get Account Policy Information"
        Me.chkGetAccountPolicyInfo.UseVisualStyleBackColor = True
        '
        'GroupBox5
        '
        Me.GroupBox5.Controls.Add(Me.txtOutput)
        Me.GroupBox5.Location = New System.Drawing.Point(12, 427)
        Me.GroupBox5.Name = "GroupBox5"
        Me.GroupBox5.Size = New System.Drawing.Size(753, 197)
        Me.GroupBox5.TabIndex = 44
        Me.GroupBox5.TabStop = False
        Me.GroupBox5.Text = "WinScanX Output"
        '
        'txtOutput
        '
        Me.txtOutput.BackColor = System.Drawing.SystemColors.Window
        Me.txtOutput.Font = New System.Drawing.Font("Courier New", 9.0!, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, CType(0, Byte))
        Me.txtOutput.Location = New System.Drawing.Point(12, 19)
        Me.txtOutput.Multiline = True
        Me.txtOutput.Name = "txtOutput"
        Me.txtOutput.ReadOnly = True
        Me.txtOutput.ScrollBars = System.Windows.Forms.ScrollBars.Vertical
        Me.txtOutput.Size = New System.Drawing.Size(726, 163)
        Me.txtOutput.TabIndex = 45
        '
        'ProgressBar1
        '
        Me.ProgressBar1.Location = New System.Drawing.Point(12, 643)
        Me.ProgressBar1.Name = "ProgressBar1"
        Me.ProgressBar1.Size = New System.Drawing.Size(753, 23)
        Me.ProgressBar1.TabIndex = 46
        '
        'btnStartScan
        '
        Me.btnStartScan.Font = New System.Drawing.Font("Arial", 9.0!, System.Drawing.FontStyle.Bold, System.Drawing.GraphicsUnit.Point, CType(0, Byte))
        Me.btnStartScan.Location = New System.Drawing.Point(451, 30)
        Me.btnStartScan.Name = "btnStartScan"
        Me.btnStartScan.Size = New System.Drawing.Size(100, 23)
        Me.btnStartScan.TabIndex = 41
        Me.btnStartScan.Text = "Start Scan"
        Me.btnStartScan.UseVisualStyleBackColor = True
        '
        'btnClear
        '
        Me.btnClear.Font = New System.Drawing.Font("Arial", 9.0!, System.Drawing.FontStyle.Bold, System.Drawing.GraphicsUnit.Point, CType(0, Byte))
        Me.btnClear.Location = New System.Drawing.Point(557, 30)
        Me.btnClear.Name = "btnClear"
        Me.btnClear.Size = New System.Drawing.Size(100, 23)
        Me.btnClear.TabIndex = 42
        Me.btnClear.Text = "Clear"
        Me.btnClear.UseVisualStyleBackColor = True
        '
        'LinkLabel1
        '
        Me.LinkLabel1.AutoSize = True
        Me.LinkLabel1.Font = New System.Drawing.Font("Arial", 9.0!, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, CType(0, Byte))
        Me.LinkLabel1.Location = New System.Drawing.Point(593, 674)
        Me.LinkLabel1.Name = "LinkLabel1"
        Me.LinkLabel1.Size = New System.Drawing.Size(172, 15)
        Me.LinkLabel1.TabIndex = 47
        Me.LinkLabel1.TabStop = True
        Me.LinkLabel1.Tag = ""
        Me.LinkLabel1.Text = "http://www.windowsaudit.com/"
        '
        'OpenFileDialog1
        '
        Me.OpenFileDialog1.FileName = "OpenFileDialog1"
        '
        'btnViewReports
        '
        Me.btnViewReports.Font = New System.Drawing.Font("Arial", 9.0!, System.Drawing.FontStyle.Bold, System.Drawing.GraphicsUnit.Point, CType(0, Byte))
        Me.btnViewReports.Location = New System.Drawing.Point(663, 30)
        Me.btnViewReports.Name = "btnViewReports"
        Me.btnViewReports.Size = New System.Drawing.Size(100, 23)
        Me.btnViewReports.TabIndex = 43
        Me.btnViewReports.Text = "View Reports"
        Me.btnViewReports.UseVisualStyleBackColor = True
        '
        'WinScanX
        '
        Me.AutoScaleDimensions = New System.Drawing.SizeF(6.0!, 13.0!)
        Me.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font
        Me.BackColor = System.Drawing.SystemColors.GradientInactiveCaption
        Me.ClientSize = New System.Drawing.Size(778, 698)
        Me.Controls.Add(Me.btnViewReports)
        Me.Controls.Add(Me.LinkLabel1)
        Me.Controls.Add(Me.btnClear)
        Me.Controls.Add(Me.btnStartScan)
        Me.Controls.Add(Me.ProgressBar1)
        Me.Controls.Add(Me.GroupBox5)
        Me.Controls.Add(Me.GroupBox2)
        Me.Controls.Add(Me.GroupBox1)
        Me.FormBorderStyle = System.Windows.Forms.FormBorderStyle.FixedSingle
        Me.Icon = CType(resources.GetObject("$this.Icon"), System.Drawing.Icon)
        Me.MaximizeBox = False
        Me.Name = "WinScanX"
        Me.Text = "WinScanX GUI | http://www.windowsaudit.com/"
        Me.GroupBox1.ResumeLayout(False)
        Me.GroupBox1.PerformLayout()
        Me.GroupBox2.ResumeLayout(False)
        Me.GroupBox2.PerformLayout()
        Me.GroupBox3.ResumeLayout(False)
        Me.GroupBox3.PerformLayout()
        Me.GroupBox4.ResumeLayout(False)
        Me.GroupBox4.PerformLayout()
        Me.GroupBox5.ResumeLayout(False)
        Me.GroupBox5.PerformLayout()
        Me.ResumeLayout(False)
        Me.PerformLayout()

    End Sub

    Friend WithEvents GroupBox1 As System.Windows.Forms.GroupBox
    Friend WithEvents GroupBox2 As System.Windows.Forms.GroupBox
    Friend WithEvents GroupBox5 As System.Windows.Forms.GroupBox
    Friend WithEvents txtTargetInput As System.Windows.Forms.TextBox
    Friend WithEvents ProgressBar1 As System.Windows.Forms.ProgressBar
    Friend WithEvents btnUseDomain As System.Windows.Forms.Button
    Friend WithEvents btnBrowse As System.Windows.Forms.Button
    Friend WithEvents chkGetDisplayInfo As System.Windows.Forms.CheckBox
    Friend WithEvents chkGetAuditPolicyInfo As System.Windows.Forms.CheckBox
    Friend WithEvents chkGetAccountPolicyInfo As System.Windows.Forms.CheckBox
    Friend WithEvents chkGetWinVNCPasswords As System.Windows.Forms.CheckBox
    Friend WithEvents chkGetUserRightsInfo As System.Windows.Forms.CheckBox
    Friend WithEvents chkGetRAUserInfo As System.Windows.Forms.CheckBox
    Friend WithEvents chkGetUserInfo As System.Windows.Forms.CheckBox
    Friend WithEvents chkGetSNMPCommunityInfo As System.Windows.Forms.CheckBox
    Friend WithEvents chkGetSharePermissions As System.Windows.Forms.CheckBox
    Friend WithEvents chkGetShareInfo As System.Windows.Forms.CheckBox
    Friend WithEvents chkGetServiceInfo As System.Windows.Forms.CheckBox
    Friend WithEvents chkGetServerInfo As System.Windows.Forms.CheckBox
    Friend WithEvents chkGetScheduledTaskInfo As System.Windows.Forms.CheckBox
    Friend WithEvents chkGetRegistryInfo As System.Windows.Forms.CheckBox
    Friend WithEvents chkGetPatchInfo As System.Windows.Forms.CheckBox
    Friend WithEvents chkGetLoggedOnUsers As System.Windows.Forms.CheckBox
    Friend WithEvents chkGetInteractivelyLoggedOnUsers As System.Windows.Forms.CheckBox
    Friend WithEvents chkGetInstalledPrograms As System.Windows.Forms.CheckBox
    Friend WithEvents chkGetLocalGlobalGroupInfo As System.Windows.Forms.CheckBox
    Friend WithEvents chkGetAdminLocalGlobalGroupInfo As System.Windows.Forms.CheckBox
    Friend WithEvents chkGetLDAPInfo As System.Windows.Forms.CheckBox
    Friend WithEvents txtOutput As System.Windows.Forms.TextBox
    Friend WithEvents chkGuessWindowsPasswords As System.Windows.Forms.CheckBox
    Friend WithEvents chkGuessSNMPCommunityStrings As System.Windows.Forms.CheckBox
    Friend WithEvents chkPingRemoteHost As System.Windows.Forms.CheckBox
    Friend WithEvents btnStartScan As System.Windows.Forms.Button
    Friend WithEvents btnClear As System.Windows.Forms.Button
    Friend WithEvents optUseAnonymousConnection As System.Windows.Forms.RadioButton
    Friend WithEvents optUseExistingCredentials As System.Windows.Forms.RadioButton
    Friend WithEvents optSpecifyUsernamePassword As System.Windows.Forms.RadioButton
    Friend WithEvents txtUsername As System.Windows.Forms.TextBox
    Friend WithEvents txtPassword As System.Windows.Forms.TextBox
    Friend WithEvents LinkLabel1 As System.Windows.Forms.LinkLabel
    Friend WithEvents chkSaveRemoteRegistryHives As System.Windows.Forms.CheckBox
    Friend WithEvents chkGetDomainInfo As System.Windows.Forms.CheckBox
    Friend WithEvents OpenFileDialog1 As System.Windows.Forms.OpenFileDialog
    Friend WithEvents optGroup3 As System.Windows.Forms.RadioButton
    Friend WithEvents optGroup2 As System.Windows.Forms.RadioButton
    Friend WithEvents optGroup1 As System.Windows.Forms.RadioButton
    Friend WithEvents GroupBox4 As System.Windows.Forms.GroupBox
    Friend WithEvents GroupBox3 As System.Windows.Forms.GroupBox
    Friend WithEvents btnViewReports As System.Windows.Forms.Button
End Class
