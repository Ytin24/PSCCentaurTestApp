using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.IO;
using System.Net;
using System.Net.Sockets;
using System.Text;
using System.Threading;

namespace MonitoringServer
{
    class Program
    {
        static TcpListener listener;
        static Dictionary<string, ClientInfo> clients = new Dictionary<string, ClientInfo>();
        static int clientIdCounter = 1;

        static void Main(string[] args)
        {
            Console.WriteLine("������ �������...");
            listener = new TcpListener(IPAddress.Any, 5000);
            listener.Start();
            Console.WriteLine("������ ������� �� ����� 5000.");

            Thread clientListenerThread = new Thread(ListenForClients);
            clientListenerThread.Start();

            RunServerConsole();
        }

        static void ListenForClients()
        {
            while (true)
            {
                TcpClient client = listener.AcceptTcpClient();
                Console.WriteLine("����� �����������...");
                Thread clientThread = new Thread(() => HandleClient(client));
                clientThread.Start();
            }
        }

        static void HandleClient(TcpClient client)
        {
            NetworkStream stream = client.GetStream();
            string clientId = $"Client_{clientIdCounter++}";

            try
            {
                while (true)
                {
                    string message = ReadMessage(stream);
                    if (string.IsNullOrEmpty(message)) break;

                    ProcessMessage(message, clientId, client, stream);
                }
            }
            catch (Exception ex)
            {
                Console.WriteLine($"������ ��� ������ � �������� {clientId}: {ex.Message}");
            }
            finally
            {
                if (clients.ContainsKey(clientId))
                {
                    clients.Remove(clientId);
                    Console.WriteLine($"������ {clientId} ����������.");
                }
                client.Close();
            }
        }

        static string ReadMessage(NetworkStream stream)
        {
            // ������ ����� ��������� (4 �����)
            byte[] lengthBuffer = new byte[4];
            int bytesRead = stream.Read(lengthBuffer, 0, lengthBuffer.Length);

            if (bytesRead == 0) return null; // ������ ������ ����������

            int messageLength = BitConverter.ToInt32(lengthBuffer.Reverse().ToArray(), 0);

            // ������ ���������
            byte[] messageBuffer = new byte[messageLength];
            int totalBytesRead = 0;

            while (totalBytesRead < messageLength)
            {
                bytesRead = stream.Read(messageBuffer, totalBytesRead, messageLength - totalBytesRead);
                if (bytesRead == 0) break; // ������ ������ ����������
                totalBytesRead += bytesRead;
            }

            return Encoding.UTF8.GetString(messageBuffer);
        }

        static void ProcessMessage(string message, string clientId, TcpClient client, NetworkStream stream)
        {
            // ���������� ������� � ������
            string[] parts = message.Split(' ', 2, StringSplitOptions.RemoveEmptyEntries);
            string command = parts[0];
            string payload = parts.Length > 1 ? parts[1] : "";

            switch (command)
            {
                case "REGISTER":
                    string[] info = payload.Split('|');
                    string machineName = info[0];
                    string userName = info[1];
                    clients[clientId] = new ClientInfo(machineName, userName, DateTime.Now, client);
                    Console.WriteLine($"������ ���������������: {machineName} ({userName})");
                    break;

                case "HEARTBEAT":
                    if (clients.ContainsKey(clientId))
                    {
                        clients[clientId].LastActive = DateTime.Now;
                    }
                    break;

                case "SCREENSHOT":
                    SaveScreenshot(payload, clientId);
                    break;

                case "EXIT":
                    clients.Remove(clientId);
                    Console.WriteLine($"������ {clientId} ����������.");
                    break;

                default:
                    Console.WriteLine($"����������� ������� �� {clientId}: {message}");
                    break;
            }
        }

        static void SaveScreenshot(string base64Data, string clientId)
        {
            byte[] data = Convert.FromBase64String(base64Data);
            string fileName = $"screenshot_{clientId}_{DateTime.Now:yyyyMMdd_HHmmss}.jpg";
            File.WriteAllBytes(fileName, data);
            ProcessStartInfo Process_Info = new ProcessStartInfo(fileName, @"%SystemRoot%\System32\rundll32.exe % ProgramFiles %\Windows Photo Viewer\PhotoViewer.dll, ImageView_Fullscreen %1")
            {
                UseShellExecute = true,
                WorkingDirectory = Path.GetDirectoryName(fileName),
                FileName = fileName,
                Verb = "OPEN"
            };
            Process.Start(Process_Info);
            Console.WriteLine($"�������� �������: {fileName}");
        }

        static void RunServerConsole()
        {
            while (true)
            {
                Console.WriteLine("\n��������� �������:");
                Console.WriteLine("1. ������ ��������");
                Console.WriteLine("2. ��������� �������� � �������");
                Console.WriteLine("3. �����");
                Console.Write("������� �������: ");

                string input = Console.ReadLine();
                switch (input)
                {
                    case "1":
                        ShowClients();
                        break;

                    case "2":
                        RequestScreenshot();
                        break;

                    case "3":
                        Console.WriteLine("���������� ������ �������...");
                        Environment.Exit(0);
                        break;

                    default:
                        Console.WriteLine("����������� �������.");
                        break;
                }
            }
        }

        static void ShowClients()
        {
            if (clients.Count == 0)
            {
                Console.WriteLine("��� ������������ ��������.");
                return;
            }

            Console.WriteLine("\n������ ��������:");
            foreach (var client in clients)
            {
                Console.WriteLine($"{client.Key}: {client.Value.MachineName} ({client.Value.UserName}) - ��������� ����������: {client.Value.LastActive}");
            }
        }

        static void RequestScreenshot()
        {
            Console.Write("������� ID �������: ");
            string clientId = Console.ReadLine();

            if (clients.ContainsKey(clientId))
            {
                TcpClient client = clients[clientId].TcpClient;
                NetworkStream stream = client.GetStream();

                string command = "SCREENSHOT_REQUEST";
                byte[] commandBytes = Encoding.UTF8.GetBytes(command);

                // ���������� ����� �������
                byte[] lengthBytes = BitConverter.GetBytes(IPAddress.HostToNetworkOrder(commandBytes.Length));
                stream.Write(lengthBytes, 0, lengthBytes.Length);

                // ���������� ���� �������
                stream.Write(commandBytes, 0, commandBytes.Length);
                Console.WriteLine($"������ �� �������� ��������� ������� {clientId}.");
            }
            else
            {
                Console.WriteLine("������ �� ������.");
            }
        }
    }

    class ClientInfo
    {
        public string MachineName { get; }
        public string UserName { get; }
        public DateTime LastActive { get; set; }
        public TcpClient TcpClient { get; }

        public ClientInfo(string machineName, string userName, DateTime lastActive, TcpClient tcpClient)
        {
            MachineName = machineName;
            UserName = userName;
            LastActive = lastActive;
            TcpClient = tcpClient;
        }
    }
}
