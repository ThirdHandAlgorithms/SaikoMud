using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading;

using System.Net.Sockets;

namespace GameClient {
    class GameNet {
        private string host = "127.0.0.1";
        private int port = 23;

        private TcpClient clientsock;
        private NetworkStream stream;

        public String Username = "testaccount";
        public String Password = "test";

        private Thread EventHandler;
        private bool isConnected = false;

        // callbacks
        public delegate void GameNetCallback(UInt32 command, UInt32 intparam1, UInt32 intparam2, String str);

        public event GameNetCallback actioninfo;
        public event GameNetCallback roominfo;
        public event GameNetCallback mapinfo;
        public event GameNetCallback chatmsginfo;

        // commands/actions
        public const UInt32 c_run_walkforward = 0x00000005;
        public const UInt32 c_run_walkbackwards = 0x00000006;
        public const UInt32 c_run_walkleft = 0x00000007;
        public const UInt32 c_run_walkright = 0x00000008;

        public const UInt32 c_attack_start = 0x10000009;
        public const UInt32 c_attack_stop = 0x0000000a;

        public const UInt32 c_chat_say = 0x30000010;

        // responses
        public const UInt32 c_response_lastactioninfo = 0x10010000;
        public const UInt32 c_response_roominfo = 0x30020000;
        public const UInt32 c_response_asciimap = 0x10030000;

        public const UInt32 c_response_chatmessage = 0x30100000;


        public GameNet() {
            EventHandler = new Thread(new ThreadStart(ThreadProc));
        }

        public void ExtractAndProcess(ref byte[] sData) {
            String sRemainingData = "";

            int i = 0;

            UInt32 command = 0;
            command |= (UInt32)(sData[i] << 24);
            command |= (UInt32)(sData[i + 1] << 16);
            command |= (UInt32)(sData[i + 2] << 8);
            command |= (UInt32)(sData[i + 3]);
            i += 4;

            UInt32 intparam1 = 0;
            UInt32 intparam2 = 0;
            String sDataStr = "";

            bool bIsStrCommand = ((command & 0x10000000) > 0);
            bool bIntParamCommand = ((command & 0x20000000) > 0);

            if (bIntParamCommand) {
                intparam1 |= (UInt32)(sData[i] << 24);
                intparam1 |= (UInt32)(sData[i + 1] << 16);
                intparam1 |= (UInt32)(sData[i + 2] << 8);
                intparam1 |= (UInt32)(sData[i + 3]);

                i += 4;

                intparam2 |= (UInt32)(sData[i] << 24);
                intparam2 |= (UInt32)(sData[i + 1] << 16);
                intparam2 |= (UInt32)(sData[i + 2] << 8);
                intparam2 |= (UInt32)(sData[i + 3]);

                i += 4;
            }

            if (bIsStrCommand) {
                UInt32 datlen = 0;
                datlen |= (UInt32)(sData[i] << 24);
                datlen |= (UInt32)(sData[i + 1] << 16);
                datlen |= (UInt32)(sData[i + 2] << 8);
                datlen |= (UInt32)(sData[i + 3]);

                i += 4;

                if (sData.Length >= (i + datlen)) {
                    sDataStr = System.Text.Encoding.ASCII.GetString( sData, i, (int)(datlen) );

                    int newlen = (int)(sData.Length - i - datlen);
                    byte[] sCopyData = new byte[newlen];
                    Buffer.BlockCopy(sData, (int)(i + datlen), sCopyData, 0, newlen);
                    sData = sCopyData;
                }
            }

            if (command == c_response_lastactioninfo) {
                actioninfo.Invoke(command, intparam1, intparam2, sDataStr);
            } else if (command == c_response_roominfo) {
                roominfo.Invoke(command, intparam1, intparam2, sDataStr);
            } else if (command == c_response_asciimap) {
                mapinfo.Invoke(command, intparam1, intparam2, sDataStr);
            } else if (command == c_response_chatmessage) {
                chatmsginfo.Invoke(command, intparam1, intparam2, sDataStr);
            }
        }

        public void ThreadProc() {
            byte[] sLargeBuffer = {};
            byte[] sSmallBuffer = {};

            while (isConnected) {
                try {
                    sSmallBuffer = ReceiveFromServer();
                    Int32 LastLength = sLargeBuffer.Length;
                    Array.Resize(ref sLargeBuffer, LastLength + sSmallBuffer.Length);
                    Buffer.BlockCopy(sSmallBuffer, 0, sLargeBuffer, LastLength, sSmallBuffer.Length);

                    if (sLargeBuffer.Length != 0) {
                        ExtractAndProcess(ref sLargeBuffer);
                    }
                } catch {
                }
            }
        }

        public bool Login() {
            SendToServer("/l " + Username + " " + Password + "\r\n");
            

            byte[] LastActionMessage = ReceiveFromServer();
            while (LastActionMessage.Length == 0) {
                LastActionMessage = ReceiveFromServer();
            }

            if (System.Text.Encoding.ASCII.GetString(LastActionMessage) == "Logged in.\r\n") {
                return true;
            }

            return false;
        }

        public void SwitchToBinaryMode() {
            SendToServer("/binarymode_on\r\n");
        }

        public Byte[] ReceiveFromServer() {
            Byte[] data = new Byte[1024];

            // String to store the response ASCII representation.
            Byte[] responseData = {};

            // Read the first batch of the TcpServer response bytes.
            Int32 bytes = 0;

            if (stream.DataAvailable) {
                bytes = stream.Read(data, 0, data.Length);
                while (bytes > 0) {
                    Int32 LastLength = responseData.Length;
                    Array.Resize(ref responseData, LastLength + bytes);
                    Buffer.BlockCopy(data, 0, responseData, LastLength, bytes);

                    if (stream.DataAvailable) {
                        bytes = stream.Read(data, 0, data.Length);
                    } else {
                        bytes = 0;
                    }
                }
            }

            return responseData;
        }

        public void SendToServer(String message) {
            // expect message to be ansistring...

            Byte[] data = System.Text.Encoding.UTF8.GetBytes(message);
            stream.Write(data, 0, data.Length);
        }

        public void SendBinToServer(UInt32 command, UInt32 intparam1, UInt32 intparam2, String message) {
            bool bIsStrCommand = ((command & 0x10000000) > 0);
            bool bIntParamCommand = ((command & 0x20000000) > 0);

            long c = 4;
            if (bIntParamCommand) {
                c += 8;
            }

            if (bIsStrCommand) {
                c += 4;

                c += message.Length;
            }

            Byte[] data = new Byte[c];

            int i = 0;
            data[i] = (Byte)((command & 0xFF000000) >> 24);
            data[i+1] = (Byte)((command & 0x00FF0000) >> 16);
            data[i+2] = (Byte)((command & 0x0000FF00) >> 8);
            data[i+3] = (Byte)(command & 0x000000FF);

            i += 4;

            if (bIntParamCommand) {
                data[i] = (Byte)((intparam1 & 0xFF000000) >> 24);
                data[i + 1] = (Byte)((intparam1 & 0x00FF0000) >> 16);
                data[i + 2] = (Byte)((intparam1 & 0x0000FF00) >> 8);
                data[i + 3] = (Byte)(intparam1 & 0x000000FF);

                i += 4;

                data[i] = (Byte)((intparam2 & 0xFF000000) >> 24);
                data[i + 1] = (Byte)((intparam2 & 0x00FF0000) >> 16);
                data[i + 2] = (Byte)((intparam2 & 0x0000FF00) >> 8);
                data[i + 3] = (Byte)(intparam2 & 0x000000FF);

                i += 4;
            }

            if (bIsStrCommand) {
                long strlen = message.Length;

                data[i] = (Byte)((strlen & 0xFF000000) >> 24);
                data[i + 1] = (Byte)((strlen & 0x00FF0000) >> 16);
                data[i + 2] = (Byte)((strlen & 0x0000FF00) >> 8);
                data[i + 3] = (Byte)(strlen & 0x000000FF);

                i += 4;

                Byte[] bytemsg = System.Text.Encoding.UTF8.GetBytes(message);
                Buffer.BlockCopy(bytemsg, 0, data, i, bytemsg.Length);
            }

            stream.Write(data, 0, data.Length);
        }

        public void Connect() {
            try {
                isConnected = false;

                clientsock = new TcpClient(host, port);
                stream = clientsock.GetStream();

                Byte[] data = ReceiveFromServer();
                while (data.Length == 0) {
                    data = ReceiveFromServer();
                }

                if (!Login()) {
                    throw new Exception("Cannot login");
                }

                isConnected = true;

                EventHandler.Start();

                SwitchToBinaryMode();

            } catch (ArgumentNullException e) {
            } catch (SocketException e) {
            }
        }

        public void Disconnect() {
            if (EventHandler.ThreadState == ThreadState.Running) {
                isConnected = false;
                EventHandler.Join();
            }

            if (stream != null) {
                stream.Close();
                clientsock.Close();
            }
        }
    }
}
