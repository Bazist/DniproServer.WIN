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

namespace BigDocClient
{
    public class BigDocQuery
    {
        internal BigDocQuery(BigDoc db)
        {
            _db = db;
            _client = db.Client;
        }

        private BigDoc _db;
        private BigDocClient _client;

        public BigDocQuery GetWhere(string json)
        {
            _client.AddPacket(7, json);

            return this;
        }

        public BigDocQuery AndWhere(string json)
        {
            _client.AddPacket(9, json);

            return this;
        }

        public BigDocQuery OrWhere(string json)
        {
            _client.AddPacket(11, json);

            return this;
        }

        public PartDoc[] Select(string json)
        {
            PartDoc[] partDocs;

            uint retSize;
            byte[] bytes = _client.CallMethod(14, json, out retSize);

            int i = 4;
            uint count = BitConverter.ToUInt32(bytes, i);

            //count
            i += 4;

            partDocs = new PartDoc[count];

            for (int j = 0; j < count; j++)
            {
                ushort len = BitConverter.ToUInt16(bytes, i);

                //len
                i += 2;

                partDocs[j].Json = _client.Encoding.GetString(bytes, i, len);

                //string
                i += len;
            }

            return partDocs;
        }

        public uint Count()
        {
            uint retSize;
            byte[] bytes = _client.CallMethod(15, "", out retSize);

            return BitConverter.ToUInt32(bytes, 4);
        }

        public BigDocQuery Take(uint count)
        {
            _client.AddPacket(17, "", count);

            return this;
        }

        public BigDocQuery Skip(uint count)
        {
            _client.AddPacket(18, "", count);

            return this;
        }

        //public BigDocQuery Sort<T>(T template, bool isAsc = true)
        //{
        //    _client.AddPacket(22, Serialization.SerializeChanges(template), isAsc ? (uint)1 : (uint)0);

        //    return this;
        //}

        public BigDocQuery Sort(string json, bool isAsc = true)
        {
            _client.AddPacket(22, json, isAsc ? (uint)1 : (uint)0);

            return this;
        }

        public uint Sum(string json)
        {
            uint retSize;
            byte[] bytes = _client.CallMethod(19, json, out retSize);

            return BitConverter.ToUInt32(bytes, 4);
        }

        public BigDocQuery Distinct(string json)
        {
            _client.AddPacket(20, json);

            return this;
        }

        public BigDocQuery Join<T>(T template1,
                                   T template2,
                                   string collName = null)
        {
            string json1 = Serialization.SerializeChanges(template1, null, true);
            string json2 = Serialization.SerializeChanges(template2, null, true);

            return Join(json1, json2, collName);
        }

        public BigDocQuery Join(string json1,
                                string json2,
                                string collName = null)
        {
            StringBuilder sb = new StringBuilder();
            sb.Append(json1);
            sb.Append(' ');
            sb.Append(json2);

            _client.AddPacket(21, sb.ToString(), (uint)json1.Length, _db.GetCollID(collName));

            return this;
        }

        public T[] Select<T>(string json = null,
                             uint maxDepth = Serialization.MaxDepth,
                             bool isDistinct = false) where T : new()
        {
            if (json == null)
            {
                json = Serialization.SerializeSchema(typeof(T), maxDepth);
            }

            uint retSize;
            byte[] bytes = _client.CallMethod(14,
                                              json,
                                              out retSize,
                                              isDistinct ? (uint)1 : (uint)0);

            int i = 4; //header
            uint count = BitConverter.ToUInt32(bytes, i);

            //count
            i += 4;

            T[] docs = new T[count];

            for (int j = 0; j < count; j++)
            {
                ushort len = BitConverter.ToUInt16(bytes, i);

                //len
                i += 2;

                docs[j] = new T();

                docs[j] = Serialization.DeserializeProxy<T>(docs[j], _client.Encoding.GetString(bytes, i, len), null, _db.GetBlobValue);

                //string
                i += len;
            }

            return docs;
        }

        public T[] Select<T>(T[] docs,
                             string json = null,
                             bool isDistinct = false)
        {
            if (json == null)
            {
                json = Serialization.SerializeSchema(typeof(T), Serialization.MaxDepth);
            }

            uint retSize;
            byte[] bytes = _client.CallMethod(14,
                                              json,
                                              out retSize,
                                              isDistinct ? (uint)1 : (uint)0);

            int i = 4; //header
            uint count = BitConverter.ToUInt32(bytes, i);

            //count
            i += 4;

            for (int j = 0; j < count; j++)
            {
                ushort len = BitConverter.ToUInt16(bytes, i);

                //len
                i += 2;

                docs[j] = Serialization.DeserializeProxy<T>(docs[j], _client.Encoding.GetString(bytes, i, len), null, _db.GetBlobValue);

                //string
                i += len;
            }

            return docs;
        }

        public T Select<T>(object doc,
                           string json = null,
                           Load[] loads = null,
                           bool isDistinct = false,
                           Serialization.Behaviour sb = null) where T : new()
        {
            if (json == null)
            {
                if (loads == null)
                {
                    if (typeof(T) == doc.GetType())
                    {
                        json = Serialization.SerializeSchema(doc.GetType(), Serialization.MaxDepth);
                    }
                    else
                    {
                        json = Serialization.SerializeSchemaPart(doc, typeof(T));
                    }
                }
                else
                {
                    throw new FormatException();
                }
            }
            else
            {
                json = Serialization.SerializeLoads(doc, loads);
            }

            uint retSize;
            byte[] bytes = _client.CallMethod(14,
                                              json,
                                              out retSize,
                                              isDistinct ? (uint)1 : (uint)0);

            int i = 4; //header
            uint count = BitConverter.ToUInt32(bytes, i);

            //count
            i += 4;

            if (count == 1) //one element
            {
                ushort len = BitConverter.ToUInt16(bytes, i);

                //len
                i += 2;

                if (typeof(T) == doc.GetType())
                {
                    return Serialization.DeserializeProxy<T>((T)doc, _client.Encoding.GetString(bytes, i, len), sb, _db.GetBlobValue);
                }
                else
                {
                    T nestedDoc = new T();
                    return Serialization.DeserializeProxy<T>(nestedDoc, _client.Encoding.GetString(bytes, i, len), sb, _db.GetBlobValue);
                }
            }
            else if (count > 1) //many elements
            {
                StringBuilder jsons = new StringBuilder();
                jsons.Append('[');

                for (int j = 0; j < count; j++)
                {
                    ushort len = BitConverter.ToUInt16(bytes, i);

                    //len
                    i += 2;

                    jsons.Append(_client.Encoding.GetString(bytes, i, len));

                    if (j != count - 1) //not last element
                    {
                        jsons.Append(',');
                    }

                    //string
                    i += len;
                }

                jsons.Append(']');

                doc = Serialization.DeserializeProxy<T>((T)doc, jsons.ToString(), sb, _db.GetBlobValue);

            }

            return (T)doc;
        }

        public void Delete(string json,
                           JoinedEnum joinedTable = JoinedEnum.FirstTable)
        {
            _client.CallMethod(23, json, (uint)joinedTable);
        }

        public void Update(string json,
                           JoinedEnum joinedTable = JoinedEnum.FirstTable)
        {
            _client.CallMethod(24, json, (uint)joinedTable);
        }

        public void Update<T>(T doc,
                              string json = null,
                              Change[] changes = null,
                              JoinedEnum joinedTable = JoinedEnum.FirstTable)
        {
            if (json == null)
            {
                json = Serialization.SerializeChanges(doc, changes);
            }
            else
            {
                json = Serialization.SerializeProxy<T>(doc, json);
            }

            _client.CallMethod(24, json, (uint)joinedTable);
        }

        public void Insert(string json,
                           JoinedEnum joinedTable = JoinedEnum.FirstTable)
        {
            _client.CallMethod(26, json, (uint)joinedTable);
        }

        public void Insert<T>(T doc,
                              string json,
                              JoinedEnum joinedTable = JoinedEnum.FirstTable)
        {
            if (json == null)
            {
                json = Serialization.SerializeSchema(typeof(T), Serialization.MaxDepth);
            }
            else
            {
                json = Serialization.SerializeProxy<T>(doc, json);
            }

            _client.CallMethod(26, json, (uint)joinedTable);
        }

        public void Insert<T>(T doc,
                              Change[] changes,
                              JoinedEnum joinedTable = JoinedEnum.FirstTable)
        {
            string json = Serialization.SerializeChanges(doc, changes);

            _client.CallMethod(26, json, (uint)joinedTable);
        }

        public void Print(string json)
        {
            PartDoc[] partDocs = Select(json);

            foreach (PartDoc pd in partDocs)
            {
                Console.WriteLine(pd.Json);
            }
        }

        public uint GetFirstID()
        {
            byte[] bytes = _client.CallMethod(36);

            return BitConverter.ToUInt32(bytes, 4);
        }

        public string Value<T>(T template)
        {
            return Value(Serialization.SerializeChanges(template, null, true));
        }

        public string Value(string json)
        {
            uint retSize;
            byte[] bytes = _client.CallMethod(20,
                                              json,
                                              out retSize);

            int i = 4; //header
            uint count = BitConverter.ToUInt32(bytes, i);

            //count
            i += 4;

            if (count == 1) //one element
            {
                ushort len = BitConverter.ToUInt16(bytes, i);

                //len
                i += 2;

                return _client.Encoding.GetString(bytes, i, len);
            }
            else if (count > 1) //many elements
            {
                throw new Exception("Multiply values");
            }
            else
            {
                return null;
            }
        }

        public BigDocQuery GetAll()
        {
            _client.AddPacket(25, "");

            return this;
        }
    }
}
