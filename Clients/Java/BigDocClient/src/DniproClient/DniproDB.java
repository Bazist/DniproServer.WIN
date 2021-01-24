/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */
/**
 *
 * @author Slavik
 */
package DniproClient;

import java.net.*;
import java.io.*;

public class DniproDB {

    private final String _host;
    private final int _port;
    
    public DniproDB(String host,
                    int port) {
        
        _host = host;
        _port = port;
    }
    
    public String Exec(String query)
    {
        try {
            InetAddress address = InetAddress.getByName(_host);
            
            Socket socket = new Socket(address, _port);
            
            DataOutputStream outToServer = new DataOutputStream(socket.getOutputStream());
            BufferedReader inFromServer = new BufferedReader(new InputStreamReader(socket.getInputStream()));  
            
            //send to server
            outToServer.writeBytes(query);
            
            String result = inFromServer.readLine();
            
            socket.close();
            
            return result;
        }
        catch (Exception g) {
            return "Exception: " + g;
        }
    }
    
    //FULL ORM VERSION WILL BE LATER
    /*
    #region Constructors

        public DniproDB(string host,
                        int port,
                        Encoding encoding = null)
        {
            if (_client == null)
            {
                _client = new DniproClient(host, port, encoding);
                _client.Open();
            }
        }

        #endregion

        #region Members

        private DniproClient _client = null;
        private string _defCollName;

        #endregion

        #region Properties

        public string DefCollName
        {
            get
            {
                return _defCollName;
            }

            set
            {
                SetDefColl(value);
            }
        }

        public Encoding Encoding
        {
            get
            {
                return _client.Encoding;
            }

            set
            {
                _client.Encoding = value;
            }
        }

        internal DniproClient Client
        {
            get
            {
                return _client;
            }
        }

        public DniproDB this[string collName]
        {
            get
            {
                return this;
            }
        }

        #endregion

        #region Classes

        #endregion

        #region Methods

        #region Trans

        public void BeginTran(TranType tranType = TranType.ReadCommited)
        {
            _client.OpenSession();

            _client._useExistingSession = true;

            byte[] bytes = _client.CallMethod(27, "", Convert.ToByte(tranType));

            _client.TranID = BitConverter.ToUInt32(bytes, 4);
        }

        public void RollbackTran()
        {
            _client.CallMethod(28);

            _client.TranID = 0;

            _client.CloseSession();

            _client._useExistingSession = false;
        }

        public void CommitTran()
        {
            _client.CallMethod(29);

            _client.TranID = 0;

            _client.CloseSession();

            _client._useExistingSession = false;
        }

        #endregion

        #region Selects

        public void SetDefColl(string collName)
        {

        }

        public byte GetCollID(string collName)
        {
            return 0;
        }

        public DniproQuery GetWhere(string json)
        {
            DniproQuery dq = new DniproQuery(this);

            return dq.GetWhere(json);
        }

        public DniproQuery GetWhereElems(string json)
        {
            DniproQuery dq = new DniproQuery(this);

            return dq.GetWhereElems(json);
        }

        public DniproQuery GetAll()
        {
            DniproQuery dq = new DniproQuery(this);

            return dq.GetAll();
        }

        public string GetPartDoc(string json, uint docID)
        {
            uint retSize;
            byte[] bytes = _client.CallMethod(6, json, out retSize, docID);

            return _client.Encoding.GetString(bytes, 6, (int)retSize - 6);
        }

        public IEnumerable<PartDoc> GetPartDocs(IEnumerable<PartDoc> partDocs)
        {
            uint retSize;
            byte[] bytes = _client.CallMethods(6, partDocs, out retSize);

            int i = 4;
            var en = partDocs.GetEnumerator();
            while (en.MoveNext())
            {
                ushort len = BitConverter.ToUInt16(bytes, i);

                //len
                i += 2;

                en.Current.SetJson(_client.Encoding.GetString(bytes, i, len));

                //string
                i += len;
            }

            return partDocs;
        }

        public T GetPartDoc<T>(T doc, Load load, uint docID)
        {
            return GetPartDoc<T>(doc, new Load[] { load }, docID);
        }

        public T GetPartDoc<T>(T doc, Load[] loads, uint docID)
        {
            string json = Serialization.SerializeLoads(doc, loads);

            uint retSize;
            byte[] bytes = _client.CallMethod(6, json, out retSize, docID);

            json = _client.Encoding.GetString(bytes, 6, (int)retSize - 6);

            return Serialization.DeserializeProxy(doc, json, null, GetBlobValue);
        }

        public T GetPartDoc<T>(T doc, string json, uint docID)
        {
            uint retSize;
            byte[] bytes = _client.CallMethod(6, json, out retSize, docID);

            json = _client.Encoding.GetString(bytes, 6, (int)retSize - 6);

            return Serialization.DeserializeProxy(doc, json, null, GetBlobValue);
        }

        public T GetPartDoc<T>(uint docID) where T : new()
        {
            string json = Serialization.SerializeSchema(typeof(T));

            uint retSize;
            byte[] bytes = _client.CallMethod(6, json, out retSize, docID);

            json = _client.Encoding.GetString(bytes, 6, (int)(retSize - 6));

            T doc = new T();

            return Serialization.DeserializeProxy<T>(doc, json, null, GetBlobValue);
        }

        public int[] GetDocsByAttr(string json)
        {
            uint retSize;
            byte[] resp = _client.CallMethod(2, json, out retSize);

            int[] result = new int[retSize / 4];
            Buffer.BlockCopy(resp, 0, result, 0, result.Length);

            return result;
        }

        #endregion

        #region AddDoc

        public uint AddDoc(object doc)
        {
            string json = Serialization.SerializeChanges(doc, new Change[] { Change.Create(doc, ChangeEnum.UpdateRecursively) }, AddBlobValue);

            byte[] bytes = _client.CallMethod(1, json);

            return BitConverter.ToUInt32(bytes, 4);
        }

        public uint AddDoc<T>(T doc, string json = null)
        {
            if (json == null)
            {
                json = Serialization.SerializeChanges(doc, new Change[] { Change.Create(doc, ChangeEnum.UpdateRecursively) }, AddBlobValue);
            }
            else
            {
                json = Serialization.SerializeProxy<T>(doc, json, null, AddBlobValue);
            }

            byte[] bytes = _client.CallMethod(1, json);

            return BitConverter.ToUInt32(bytes, 4);
        }

        public void AddDocs<T>(T[] docs)
        {
            string[] jsons = new string[docs.Length];

            for (int i = 0; i < docs.Length; i++)
            {
                jsons[i] = Serialization.SerializeChanges(docs[i], new Change[] { Change.Create(docs[i], ChangeEnum.UpdateRecursively) }, AddBlobValue);
            }

            _client.CallMethods(1, jsons);
        }

        public void AddDoc(string json)
        {
            _client.CallMethod(1, json);
        }

        public void AddDocs(IEnumerable<string> jsons)
        {
            _client.CallMethods(1, jsons);
        }

        public void AddDoc(PartDoc partDoc)
        {
            _client.CallMethod(1, partDoc.Json);
        }

        public void AddDocs(IEnumerable<PartDoc> partDocs)
        {
            _client.CallMethods(1, partDocs);
        }

        #endregion

        #region UpdPartDoc

        public uint UpdPartDoc<T>(T doc, uint docID)
        {
            string json = Serialization.SerializeChanges(doc, new Change[] { Change.Create(doc, ChangeEnum.UpdateRecursively) }, AddBlobValue);

            byte[] bytes = _client.CallMethod(4, json, docID);

            return BitConverter.ToUInt32(bytes, 4);
        }

        public uint UpdPartDoc(string json, uint docID)
        {
            byte[] bytes = _client.CallMethod(4, json, docID);

            return BitConverter.ToUInt32(bytes, 4);
        }

        public uint UpdPartDoc<T>(T doc, Change change, uint docID)
        {
            return UpdPartDoc<T>(doc, new Change[] { change }, docID);
        }

        public uint UpdPartDoc<T>(T doc, Change[] changes, uint docID)
        {
            string json = Serialization.SerializeChanges(doc, changes, AddBlobValue);

            byte[] bytes = _client.CallMethod(4, json, docID);

            return BitConverter.ToUInt32(bytes, 4);
        }

        public uint UpdPartDoc<T>(T doc, string json, uint docID)
        {
            json = Serialization.SerializeProxy<T>(doc, json, null, AddBlobValue);

            byte[] bytes = _client.CallMethod(4, json, docID);

            return BitConverter.ToUInt32(bytes, 4);
        }

        public void UpdPartDocs(IEnumerable<PartDoc> partDocs)
        {
            _client.CallMethods(4, partDocs);
        }

        //public void UpdDocs<T>(T[] docs, uint docID)
        //{
        //    string[] jsons = new string[docs.Length];

        //    for (int i = 0; i < docs.Length; i++)
        //    {
        //        jsons[i] = Serialization.Serialize(docs[i]);
        //    }

        //    _client.CallMethods(4, jsons);
        //}

        #endregion

        #region InsPartDoc

        public void InsPartDoc(string json, uint docID)
        {
            _client.CallMethod(3, json, docID);
        }

        public void InsPartDocs(IEnumerable<PartDoc> partDocs)
        {
            _client.CallMethods(3, partDocs);
        }

        #endregion

        #region DelPartDoc

        public void DelPartDoc(string json)
        {
            _client.CallMethod(5, json);
        }

        public void DelPartDocs(IEnumerable<PartDoc> partDocs)
        {
            _client.CallMethods(5, partDocs);
        }

        #endregion

        #region Administration

        public void Clear()
        {
            _client.CallMethod(16);
        }

        #endregion

        #region Interpreter

        public string Exec(string query)
        {
            uint retSize;
            byte[] bytes = _client.CallMethod(30, query, out retSize);

            return _client.Encoding.GetString(bytes, 4, (int)retSize - 4);
        }

        public string ExecScript(string filePath)
        {
            using (StreamReader sr = new StreamReader(filePath))
            {
                string script = sr.ReadToEnd();

                return Exec(script);
            }
        }

        #endregion

        #region Blob

        public string AddBlobValue(byte[] value)
        {
            uint retSize;
            byte[] bytes = _client.CallMethod(31, value, out retSize);

            return _client.Encoding.GetString(bytes, 4, (int)retSize - 4);
        }

        public byte[] GetBlobValue(string label)
        {
            uint retSize;
            byte[] bytes = _client.CallMethod(32, label, out retSize);

            byte[] result = new byte[retSize - 4];

            Array.Copy(bytes, 4, result, 0, retSize - 4);

            return result;
        }

        #endregion

        #endregion
    */
}

