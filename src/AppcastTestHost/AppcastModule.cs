using System;
using System.IO;
using Nancy;
using Nancy.Responses;

namespace AppcastTestHost
{
    public class AppcastModule : NancyModule
    {
        private static string AppDirectory
        {
            get
            {
                return Path.Combine(Path.GetDirectoryName(System.Reflection.Assembly.GetExecutingAssembly().Location));
            }
        }

        public AppcastModule()
        {
            Get["/{path}/{document}.{extension}"] = _p =>
            {
                try
                {
                    return GetFile(Path.Combine(AppDirectory, _p.path), _p.document + "." + _p.extension);
                }
                catch (Exception ex)
                {
                    Console.WriteLine("[500] " + ex.Message);
                    return new Response
                    {
                        StatusCode = HttpStatusCode.InternalServerError,
                        ReasonPhrase = ex.Message
                    };
                }
            };
        }

        private Response GetFile(string directory, string documentName)
        {
            // Set full path
            var fullPath = Path.Combine(directory, documentName);

            // Ensure file exists
            if (!File.Exists(fullPath))
            {
                Console.WriteLine("[404] " + fullPath);
                return new Response
                {
                    StatusCode = HttpStatusCode.NotFound,
                    ReasonPhrase = "File not found: " + documentName
                };
            }

            // Return file
            Console.WriteLine("[200] " + fullPath);
            //return new StreamResponse(() => GenerateStreamFromString(File.ReadAllText(fullPath)), MimeTypes.GetMimeType(fullPath)).AsAttachment(documentName);
            
            var file = new FileStream(fullPath, FileMode.Open);
 
            return new StreamResponse(() => file, MimeTypes.GetMimeType(fullPath));
        }

        private Stream GenerateStreamFromString(string s)
        {
            var stream = new MemoryStream();
            var writer = new StreamWriter(stream);
            writer.Write(s);
            writer.Flush();
            stream.Position = 0;
            return stream;
        }
    }
}
