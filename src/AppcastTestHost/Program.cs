using System;
using System.Collections.Generic;
using Nancy.Hosting.Self;

namespace AppcastTestHost
{
    class Program
    {
        static void Main(string[] args)
        {
            try
            {
                // Set URIs to bind to
                var uriList = new List<Uri>
                {
                    new Uri("http://localhost:8000")
                };

                // Dynamically run services as the user that is executing this application
                var username = (string.IsNullOrEmpty(Environment.UserDomainName)
                    ? Environment.MachineName
                    : Environment.UserDomainName)
                               + "\\" + Environment.UserName;
                var hostConfig = new HostConfiguration
                {
                    UrlReservations = new UrlReservations
                    {
                        CreateAutomatically = true,
                        User = username
                    }
                };

                // Start the Nancy host
                using (var host = new NancyHost(hostConfig, uriList.ToArray()))
                {
                    // Finds any public services in the module and starts them
                    host.Start();
                    Console.WriteLine("The application is running on: ");
                    foreach (var uri in uriList)
                    {
                        Console.WriteLine(uri);
                    }

                    // Wait for it to end
                    Console.WriteLine("Service is running. Press [Enter] to close the host.");
                    Console.ReadLine();
                }
            }
            catch (Exception ex)
            {
                Console.WriteLine(ex);
                Console.WriteLine("Press any key to exit.");
                Console.ReadKey();
            }
        }
    }
}
