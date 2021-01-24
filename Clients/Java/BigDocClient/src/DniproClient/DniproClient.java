/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */
package DniproClient;

/**
 *
 * @author Slavik
 */
public class DniproClient {

    public DniproClient(String host, int port, String encoding) {
        _host = host;
        _port = port;

        /*
            _unBuffer = Marshal.AllocHGlobal(10000000);
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
         */
    }

    public int _tranID;
    public String _encoding;
    private String _host;
    private int _port;
    int _dniproPacketSize;

    /*
        IntPtr _unBuffer;

        IPEndPoint _remoteEP = null;
        Socket _sender = null;

        Encoding _encoding;

        public byte[] _buffer = new byte[50000000];
        public int _bufferPos = 4;
     */
    private DniproPacket _packet;

    /*
        public Encoding Encoding;
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
     */
    public void Open() {
        /*
            IPHostEntry ipHostInfo = Dns.Resolve(_host);
            IPAddress ipAddress = ipHostInfo.AddressList[0];

            _remoteEP = new IPEndPoint(ipAddress, _port);
         */

        //Console.WriteLine("Socket connected to {0}",
        //    _sender.RemoteEndPoint.ToString());
    }

    public void AddPacket(int methodType,
            String json,
            int tag1,
            int tag2) {
        _packet.MethodType = methodType;
        _packet.Tag1 = tag1;
        _packet.Tag2 = tag2;

        AddPacket(json);
    }

    private void AddPacket(String json) {
        /*
            _packet.TranID = _tranID;

            _packet.QuerySize = (uint)json.Length + 1;

            Marshal.StructureToPtr(_packet, _unBuffer, false);

            Marshal.Copy(_unBuffer, _buffer, _bufferPos, _dniproPacketSize);

            _bufferPos += _dniproPacketSize;

            Array.Copy(_encoding.GetBytes(json), 0, _buffer, _bufferPos, json.Length);

            _bufferPos += (int)_packet.QuerySize;

            _buffer[_bufferPos - 1] = 0;
            
         */
    }

    public byte[] SendPackets(int retSize) //out
    {
        /*
            _sender = new Socket(AddressFamily.InterNetwork,
                                 SocketType.Stream, ProtocolType.Tcp);

            //full length
            _buffer[3] = (byte)(_bufferPos >> 24);
            _buffer[2] = (byte)(_bufferPos >> 16);
            _buffer[1] = (byte)(_bufferPos >> 8);
            _buffer[0] = (byte)_bufferPos;

            _sender.Connect(_remoteEP);

            int bytesSent = _sender.Send(_buffer, _bufferPos, SocketFlags.DontRoute);

            retSize = (uint)_sender.Receive(_buffer);
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

            // Release the socket.
            _sender.Shutdown(SocketShutdown.Both);
            _sender.Close();

            _bufferPos = 4;
            
            return _buffer;
         */

        return null;
    }

    public byte[] CallMethod(int methodType,
            String json,
            int tag1,
            int tag2) {
        int retSize = 0;
        return CallMethod(methodType, json, retSize, tag1, tag2); //out
    }

    public byte[] CallMethod(int methodType,
            String json,
            int retSize, //out
            int tag1,
            int tag2) {
        _packet.MethodType = methodType;
        _packet.Tag1 = tag1;
        _packet.Tag2 = tag2;

        AddPacket(json);

        return SendPackets(retSize); //out
    }

    public byte[] CallMethods(int methodType,
            Iterable<String> jsons) {
        int retSize = 0;

        return CallMethods(methodType, jsons, retSize); //out
    }

    public byte[] CallMethods(int methodType,
            Iterable<String> jsons,
            int retSize) //out
    {
        for (String json : jsons) {
            _packet.MethodType = 1;

            AddPacket(json);
        }

        return SendPackets(retSize); //out
    }

    /*
        public byte[] CallMethods(byte methodType,
                                  Iterable<PartDoc> partDocs)
        {
            int retSize;
            return CallMethods(methodType, partDocs, retSize); //out
        }

        public byte[] CallMethods(byte methodType,
                                   Iterable<PartDoc> partDocs,
                                   int retSize) //out
        {
            for (PartDoc partDoc : partDocs)
            {
                _packet.MethodType = methodType;
                _packet.Tag1 = partDoc.DocID;

                AddPacket(partDoc.Json);
            }

            return SendPackets(retSize); //out
        }
     */
    public void Close() {

        // Release unmanaged memory.
        //Marshal.FreeHGlobal(_unBuffer);
    }
}
