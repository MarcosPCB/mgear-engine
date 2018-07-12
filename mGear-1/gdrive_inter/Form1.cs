using Google;
using Google.Apis.Auth.OAuth2;
using Google.Apis.Drive.v3;
using Google.Apis.Drive.v3.Data;
using Google.Apis.Services;
using Google.Apis.Util.Store;
using Google.Apis.Download;

using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Threading;
using System.Threading.Tasks;
using System.Windows.Forms;

namespace gdrive_inter
{
    public partial class Form1 : Form
    {
        public gdrive2 g_drive;

        public Form1(gdrive2 g)
        {
            InitializeComponent();
            g_drive = g;
            g_drive.t = new Thread(new ThreadStart(g_drive.DownloadFile));
            g_drive.form = this;
            progressBar1.Maximum = (int) g_drive.final;
            label1.Text = string.Format("Downloading file: {0} - {1}/{2}", g_drive.filen, g_drive.downloaded, g_drive.final);
            g_drive.t.Start();
        }

        private void progressBar1_Click(object sender, EventArgs e)
        {
            int d = (int) g_drive.downloaded;
            int f = (int) g_drive.final;

            progressBar1.Value = (d / f) * 100;
            progressBar1.Maximum = f;
        }
    }
}
