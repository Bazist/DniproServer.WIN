/*
# Copyright(C) 2010-2017 Viacheslav Makoveichuk (email: BigDoc@gmail.com, skype: vyacheslavm81)
# This file is part of BigDocClient.
#
# BigDocClient is free software : you can redistribute it and / or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# BigDocClient is distributed in the hope that it will be useful,
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
using System.Runtime.Serialization.Json;
using System.IO;

namespace BigDocClient
{
    public class BigDoc
    {
        #region Constructors

        public BigDoc(string host,
                        int port,
                        Encoding encoding = null)
        {
            if (_client == null)
            {
                _client = new BigDocClient(host, port, encoding);
                _client.Open();
            }
        }

        #endregion

        #region Members

        private BigDocClient _client = null;

        private Dictionary<string, uint> _collIDs = new Dictionary<string,uint>();
        
        #endregion

        #region Properties

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

        internal BigDocClient Client
        {
            get
            {
                return _client;
            }
        }

        public BigDoc this[string collName]
        {
            get
            {
                SetDefColl(collName);

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
            //_client.OpenSession();

            _client._useExistingSession = false; // true;

            byte[] bytes = _client.CallMethod(27, "", Convert.ToByte(tranType), _client.CurrentTranID);

            _client.PushTranID( BitConverter.ToUInt32(bytes, 4));
        }

        public void RollbackTran()
        {
            _client.CallMethod(28, "");

            _client.PopTranID();

            //_client.CloseSession();

            _client._useExistingSession = false;
        }

        public void CommitTran()
        {
            _client.CallMethod(29, "");

            _client.PopTranID();

            //_client.CloseSession();

            _client._useExistingSession = false;
        }

        #endregion

        #region Selects

        public void SetDefColl(string collName)
        {
            if (!_collIDs.ContainsKey(collName))
            {
                uint collID = GetCollID(collName);

                _collIDs.Add(collName, collID);
            }

            Client.CollID = _collIDs[collName];
        }

        public BigDocQuery GetWhere(string json)
        {
            BigDocQuery dq = new BigDocQuery(this);

            return dq.GetWhere(json);
        }

        public BigDocQuery GetWhere<T>(T template)
        {
            BigDocQuery dq = new BigDocQuery(this);

            return dq.GetWhere(Serialization.SerializeChanges(template));
        }

        public BigDocQuery GetAll()
        {
            BigDocQuery dq = new BigDocQuery(this);

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
            return GetPartDoc<T>(doc, json, null, docID);
        }

        public T GetPartDoc<T>(T doc, string json, string jsonObj, uint docID)
        {
            uint retSize;
            byte[] bytes = _client.CallMethod(6, json, out retSize, docID);

            json = _client.Encoding.GetString(bytes, 6, (int)retSize - 6);

            if (jsonObj == null)
            {
                return Serialization.DeserializeProxy(doc, json, null, GetBlobValue);
            }
            else
            {
                return Serialization.DeserializeProxy(doc, json, jsonObj, null, GetBlobValue);
            }
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
            string json = Serialization.SerializeChanges(doc, new Change[] { Change.Create(doc, ChangeEnum.UpdateRecursively) }, false, AddBlobValue);

            byte[] bytes = _client.CallMethod(1, json);

            return BitConverter.ToUInt32(bytes, 4);
        }

        public uint AddDoc<T>(T doc,
                              string json = null,
                              string jsonObj = null)
        {
            if (json == null)
            {
                json = Serialization.SerializeChanges(doc, new Change[] { Change.Create(doc, ChangeEnum.UpdateRecursively) }, false, AddBlobValue);
            }
            else
            {
                if (jsonObj == null)
                {
                    json = Serialization.SerializeProxy<T>(doc, json, null, AddBlobValue);
                }
                else
                {
                    json = Serialization.SerializeProxy<T>(doc, json, jsonObj, null, AddBlobValue);
                }
            }

            byte[] bytes = _client.CallMethod(1, json);

            return BitConverter.ToUInt32(bytes, 4);
        }

        public void AddDocs<T>(T[] docs)
        {
            string[] jsons = new string[docs.Length];

            for (int i = 0; i < docs.Length; i++)
            {
                jsons[i] = Serialization.SerializeChanges(docs[i], new Change[] { Change.Create(docs[i], ChangeEnum.UpdateRecursively) }, false, AddBlobValue);
            }

            _client.CallMethods(1, jsons);
        }

        public uint AddDoc(string json)
        {
            byte[] bytes = _client.CallMethod(1, json);

            return BitConverter.ToUInt32(bytes, 4);
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
            string json = Serialization.SerializeChanges(doc, new Change[] { Change.Create(doc, ChangeEnum.UpdateRecursively) }, false, AddBlobValue);

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
            string json = Serialization.SerializeChanges(doc, changes, false, AddBlobValue);

            byte[] bytes = _client.CallMethod(4, json, docID);

            return BitConverter.ToUInt32(bytes, 4);
        }

        public uint UpdPartDoc<T>(T doc,
                                  string json,
                                  uint docID)
        {
            return UpdPartDoc<T>(doc, json, null, docID);
        }

        public uint UpdPartDoc<T>(T doc,
                                  string json,
                                  string jsonObj,
                                  uint docID)
        {
            if (jsonObj == null)
            {
                json = Serialization.SerializeProxy<T>(doc, json, null, AddBlobValue);
            }
            else
            {
                json = Serialization.SerializeProxy<T>(doc, json, jsonObj, null, AddBlobValue);
            }

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

        public void InsPartDoc<T>(T doc,
                                  string json,
                                  uint docID)
        {
            InsPartDoc<T>(doc, json, null, docID);
        }

        public void InsPartDoc<T>(T doc,
                                  string json,
                                  string jsonObj,
                                  uint docID)
        {
            if (jsonObj == null)
            {
                json = Serialization.SerializeProxy<T>(doc, json, null, AddBlobValue);
            }
            else
            {
                json = Serialization.SerializeProxy<T>(doc, json, jsonObj, null, AddBlobValue);
            }

            _client.CallMethod(3, json, docID);
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

        public void AddColl(string name)
        {
            _client.CallMethod(33, name);
        }

        public uint GetCollID(string name = null)
        {
            if (name != null)
            {
                if (!_collIDs.ContainsKey(name)) //read from db
                {
                    byte[] bytes = _client.CallMethod(34, name);

                    return BitConverter.ToUInt32(bytes, 4);
                }
                else //cached
                {
                    return _collIDs[name];
                }
            }
            else //default
            {
                return Client.CollID;
            }
        }

        public void DelColl(string name)
        {
            _client.CallMethod(35, name);
        }

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
    };

}
