using Google;
using Google.Apis.Auth.OAuth2;
using Google.Apis.Drive.v3;
using Google.Apis.Drive.v3.Data;
using Google.Apis.Services;
using Google.Apis.Util.Store;
using Google.Apis.Download;

using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Text;
using System.Threading;
using System.Threading.Tasks;
using System.Windows.Forms;

namespace gdrive_inter
{
    public class gdrive2
    {
        internal static string[] Scopes = { DriveService.Scope.Drive };
        internal static string ApplicationName = "Drive API .NET Quickstart";

        static string fileId;
        static DriveService Service;

        public string prj_name, user_name, password;

        public string filen;

        public Thread t;

        public long downloaded;
        public long? final;

        public Form1 form;

        public gdrive2()
        {
            UserCredential credential;

            using (var stream = new FileStream("client_secret.json", FileMode.Open, FileAccess.Read))
            {
                string credPath = System.Environment.GetFolderPath(
                    System.Environment.SpecialFolder.Personal);
                credPath = Path.Combine(credPath, ".credentials/drive-dotnet-quickstart.json");

                credential = GoogleWebAuthorizationBroker.AuthorizeAsync(
                    GoogleClientSecrets.Load(stream).Secrets,
                    Scopes,
                    "user",
                    CancellationToken.None,
                    new FileDataStore(credPath, true)).Result;
            }

            Service = new DriveService(new BaseClientService.Initializer()
            {
                HttpClientInitializer = credential,
                ApplicationName = ApplicationName,

            });

            // Define parameters of request.
            FilesResource.ListRequest listRequest = Service.Files.List();
            listRequest.PageSize = 10;
            listRequest.Fields = "nextPageToken, files(id, name, size)";

            // List files.
            IList<Google.Apis.Drive.v3.Data.File> files = listRequest.Execute().Files;
            Console.WriteLine("Files:");
            if (files != null && files.Count > 0)
            {
                foreach (var file in files)
                {
                    Console.WriteLine("{0} ({1})", file.Name, file.Id);
                    if (file.Name == "suspense.wav")
                    {
                        fileId = file.Id.ToString();
                        final = file.Size;
                        filen = file.Name;
                    }
                }
            }
            else
            {
                Console.WriteLine("No files found.");
            }
            Console.Read();
        }

        public void DownloadFile()
        {
            var request = Service.Files.Get(fileId);
            var stream2 = new System.IO.MemoryStream();

            request.MediaDownloader.ChunkSize = 1024;

            request.MediaDownloader.ProgressChanged +=
           (IDownloadProgress progress) =>
           {
               switch (progress.Status)
               {
                   case DownloadStatus.Downloading:
                       {
                           Console.WriteLine(progress.BytesDownloaded);
                           downloaded = progress.BytesDownloaded;
                           int d = (int)downloaded;
                           //int f = (int)final;

                           form.progressBar1.Invoke(new Action(() =>
                           {
                               form.progressBar1.Value = d;
                               //form.progressBar1.Maximum = f;
                           }));

                           form.label1.Invoke(new Action(() =>
                               {
                                   form.label1.Text = string.Format("Downloading file: {0} - {1}mb/{2}mb", filen, downloaded/(1024*1024), final/(1024*1024));
                               }));

                          // final = request.
                           break;
                       }
                   case DownloadStatus.Completed:
                       {
                          Console.WriteLine("Download complete.");
                           form.label2.Invoke(new Action(() =>
                               {
                                   form.label2.Text = "Download complete";
                               }));
                           break;
                       }
                   case DownloadStatus.Failed:
                       {
                           Console.WriteLine("Download failed.");
                           form.label2.Invoke(new Action(() =>
                           {
                               form.label2.Text = "Download failed";
                           }));
                           break;
                       }
               }
           };
            request.Download(stream2);
        }
    }

    static class Program
    {

        /// <summary>
        /// The main entry point for the application.
        /// </summary>
        [STAThread]
        static int Main(string[] args)
        {
            gdrive2 g = new gdrive2();

            for (int i = 0; i < args.Length;i++)
            {
                switch (args[i])
                {
                    case "-prj":
                        g.prj_name = args[i + 1];
                        break;

                    case "-user":
                        g.user_name = args[i + 1];
                        break;

                    case "-pass":
                        g.password = args[i + 1];
                        break;

                }
            }

            Application.EnableVisualStyles();
            Application.SetCompatibleTextRenderingDefault(false);
            Application.Run(new Form1(g));

            return 0;
        }
    }
}
