using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Security.Permissions;
using Microsoft.Win32;

namespace UpdVer
{
    class Program
    {
        static void Main(string[] args)
        {
            string version = null;
            int state = 0;

            //System.IO.Stream f = new System.IO.MemoryStream();

            try { version = System.IO.File.ReadAllText("version"); }

            catch(SystemException ex)
            {
                Console.WriteLine("{0}",ex.Message);
                Console.ReadKey();
                Environment.Exit(1);
            }

            //byte[] b = new byte[10];
            //f.
            /*
            if(b == null)
            {
                f.Close();
                Console.WriteLine("Empty version file");
                Console.ReadKey();
                Environment.Exit(1);
            }

            version = 

            f.Close();
            */
            for(int i = 0; i < version.Length; i++)
            {
                if((version[i] < 48 && version[i] > 57) && version[i] != 46 && version[i] != '\0')
                {
                    Console.WriteLine("Invalid version file");
                    Console.ReadKey();
                    Environment.Exit(1);
                }
            }

            RegistryPermission reg = new RegistryPermission(RegistryPermissionAccess.AllAccess, @"HKEY_LOCAL_MACHINE\\Software\\Space In a Bottle");
            try { reg.Demand(); }
            catch (System.Security.SecurityException ex) { Console.WriteLine("Error: 0x01 - {0}", ex.Message); }
            RegistryPermission reg2 = new RegistryPermission(RegistryPermissionAccess.AllAccess, @"HKEY_LOCAL_MACHINE\\Software\\WOW6432Node\\Space In a Bottle");
            try { reg.Demand(); }
            catch (System.Security.SecurityException ex) { Console.WriteLine("Error: 0x02 - {0}", ex.Message); }

            try
            {
                RegistryKey key = Registry.LocalMachine.OpenSubKey(@"Software\\Space In a Bottle", true);
                if (key != null)
                {
                    key.SetValue("Version", version);
                    state++;
                    key.Close();
                }
            }

            catch(System.Security.SecurityException ex)
            {
                Console.WriteLine("Error: 0x03 - {0}", ex.Message);
            }

            try
            {
                RegistryKey key = Registry.LocalMachine.OpenSubKey(@"Software\\WOW6432Node\\Space In a Bottle", true);
                if (key != null)
                {
                    key.SetValue("Version", version);
                    state++;
                    key.Close();
                }
            }

            catch (System.Security.SecurityException ex)
            {
                Console.WriteLine("Error: 0x04 - {0}", ex.Message);
            }


            if (state > 0)
                Console.WriteLine("Version {0} installed", version);
            else
            {
                Console.WriteLine("Could not install version on the system");
                Console.ReadKey();
                Environment.Exit(1);
            }

            Environment.Exit(0);
        }
    }
}
