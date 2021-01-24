/*
# Copyright(C) 2010-2017 Viacheslav Makoveichuk (email: dniprodb@gmail.com, skype: vyacheslavm81)
# This file is part of DniproClient.
#
# DniproClient is free software : you can redistribute it and / or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# DniproClient is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with Foobar.  If not, see <http://www.gnu.org/licenses/>.
*/

using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Runtime.InteropServices;
using System.Net;
using System.Net.Sockets;

namespace DniproClient
{
    internal class DniproClient
    {
        public DniproClient(string host, int port, Encoding encoding)
        {
            _host = host;
            _port = port;

            _unBuffer = Marshal.AllocHGlobal(1000000);
            _dniproPacketSize = Marshal.SizeOf(new DniproPacket());
            _packet = new DniproPacket();

            if (encoding == null)
            {
                _encoding = Encoding.GetEncoding("windows-1251");
            }
            else
            {
                _encoding = encoding;
            }
        }

        internal uint CurrentTranID
        {
            get
            {
                if (_tranIDs.Count > 0)
                {
                    return _tranIDs[_tranIDs.Count - 1];
                }
                else
                {
                    return 0;
                }
            }
        }

        internal void PushTranID(uint tranID)
        {
            _tranIDs.Add(tranID);
        }

        internal void PopTranID()
        { 
            _tranIDs.RemoveAt(_tranIDs.Count - 1);
        }

        internal uint CollID
        {
            get
            {
                return _collID;
            }

            set
            {
                _collID = value;
            }
        }

        private List<uint> _tranIDs = new List<uint>();

        private uint _collID;

        private string _host;
        private int _port;
        int _dniproPacketSize;
        IntPtr _unBuffer;

        IPEndPoint _remoteEP = null;
        Socket _sender = null;

        Encoding _encoding;

        public byte[] _buffer = new byte[1000000];
        public int _bufferPos = 4;
        private DniproPacket _packet;

        internal bool _useExistingSession = false;

        public Encoding Encoding
        {
            get
            {
                return _encoding;
            }

            set
            {
                _encoding = value;
            }
        }

        public void Open()
        {
            IPHostEntry ipHostInfo = Dns.Resolve(_host);
            IPAddress ipAddress = ipHostInfo.AddressList[0];

            _remoteEP = new IPEndPoint(ipAddress, _port);

            //Console.WriteLine("Socket connected to {0}",
            //    _sender.RemoteEndPoint.ToString());
        }

        public void OpenSession()
        {
            if (!_useExistingSession)
            {
                _sender = new Socket(AddressFamily.InterNetwork,
                                     SocketType.Stream,
                                     ProtocolType.Tcp);

                _sender.Connect(_remoteEP);
            }
        }

        public void CloseSession()
        {
            if (!_useExistingSession)
            {
                // Release the socket.
                _sender.Shutdown(SocketShutdown.Both);
                _sender.Close();
            }
        }

        public void AddPacket(byte methodType,
                              string json,
                              uint tag1 = 0,
                              uint tag2 = 0)
        {
            _packet.MethodType = methodType;
            _packet.Tag1 = tag1;
            _packet.Tag2 = tag2;

            AddPacket(json);
        }

        private void AddPacket(string json)
        {
            AddPacket(_encoding.GetBytes(json), (uint)json.Length + 1);
        }

        private void AddPacket(byte[] json, uint len)
        {
            _packet.TranID = CurrentTranID;
            _packet.CollID = _collID;

            _packet.QuerySize = len;

            Marshal.StructureToPtr(_packet, _unBuffer, false);

            Marshal.Copy(_unBuffer, _buffer, _bufferPos, _dniproPacketSize);

            _bufferPos += _dniproPacketSize;

            Array.Copy(json, 0, _buffer, _bufferPos, json.Length);

            _bufferPos += (int)_packet.QuerySize;

            _buffer[_bufferPos - 1] = 0;
        }

        public byte[] SendPackets(out uint retSize)
        {
            this.OpenSession();

            //full length
            _buffer[3] = (byte)(_bufferPos >> 24);
            _buffer[2] = (byte)(_bufferPos >> 16);
            _buffer[1] = (byte)(_bufferPos >> 8);
            _buffer[0] = (byte)_bufferPos;

            int bytesSent = _sender.Send(_buffer, _bufferPos, SocketFlags.DontRoute);

            retSize = (uint)_sender.Receive(_buffer);

            if (retSize >= 4)
            {
                uint totalSize = BitConverter.ToUInt32(_buffer, 0);

                if (totalSize > retSize)
                {
                    byte[] _tempBuffer = new byte[1000000];

                    do
                    {
                        //receive next portion
                        uint tempRetSize = (uint)_sender.Receive(_tempBuffer);

                        for (int i = 0; i < tempRetSize; i++, retSize++)
                        {
                            _buffer[retSize] = _tempBuffer[i];
                        }
                    }
                    while (totalSize > retSize);
                }
            
                _bufferPos = 4;

                CloseSession();
            }
            else
            {
                _bufferPos = 4;

                CloseSession();

                throw new Exception("Server is not responded.");
            }

            return _buffer;
        }

        public byte[] CallMethod(byte methodType,
                                 string json = "",
                                 uint tag1 = 0,
                                 uint tag2 = 0)
        {
            uint retSize;
            return CallMethod(methodType, json, out retSize, tag1, tag2);
        }

        public byte[] CallMethod(byte methodType,
                                  string json,
                                  out uint retSize,
                                  uint tag1 = 0,
                                  uint tag2 = 0)
        {
            _packet.MethodType = methodType;
            _packet.Tag1 = tag1;
            _packet.Tag2 = tag2;

            AddPacket(json);

            return SendPackets(out retSize);
        }

        public byte[] CallMethod(byte methodType,
                                  byte[] json,
                                  out uint retSize,
                                  uint tag1 = 0,
                                  uint tag2 = 0)
        {
            _packet.MethodType = methodType;
            _packet.Tag1 = tag1;
            _packet.Tag2 = tag2;

            AddPacket(json, (uint)json.Length);

            return SendPackets(out retSize);
        }

        public byte[] CallMethods(byte methodType,
                                   IEnumerable<string> jsons)
        {
            uint retSize;
            return CallMethods(methodType, jsons, out retSize);
        }

        public byte[] CallMethods(byte methodType,
                                   IEnumerable<string> jsons,
                                   out uint retSize)
        {
            foreach (string json in jsons)
            {
                _packet.MethodType = 1;

                AddPacket(json);
            }

            return SendPackets(out retSize);
        }

        public byte[] CallMethods(byte methodType,
                                   IEnumerable<PartDoc> partDocs)
        {
            uint retSize;
            return CallMethods(methodType, partDocs, out retSize);
        }

        public byte[] CallMethods(byte methodType,
                                   IEnumerable<PartDoc> partDocs,
                                   out uint retSize)
        {
            foreach (PartDoc partDoc in partDocs)
            {
                _packet.MethodType = methodType;
                _packet.Tag1 = partDoc.DocID;

                AddPacket(partDoc.Json);
            }

            return SendPackets(out retSize);
        }

        public void Close()
        {
            // Release unmanaged memory.
            Marshal.FreeHGlobal(_unBuffer);
        }
    }
}
