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
public class DniproQuery {

    public DniproQuery(DniproClient client) {
        _client = client;
    }

    private DniproClient _client;

    public DniproQuery GetWhere(String json) {
        _client.AddPacket(7, json, 0, 0);

        return this;
    }

    public DniproQuery GetWhereElems(String json, int docID) {
        _client.AddPacket(8, json, docID, 0);

        return this;
    }

    public DniproQuery AndWhere(String json) {
        _client.AddPacket(9, json, 0, 0);

        return this;
    }

    public DniproQuery AndWhereElems(String json, int docID) {
        _client.AddPacket(10, json, docID, 0);

        return this;
    }

    public DniproQuery OrWhere(String json) {
        _client.AddPacket(11, json, 0, 0);

        return this;
    }

    public DniproQuery OrWhereElems(String json, int docID) {
        _client.AddPacket(12, json, docID, 0);

        return this;
    }

    public PartDoc[] Select(String json) {
        /*
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
         */

        return null;
    }

    public int Count() {
        /*
            int retSize = 0;
            
            byte[] bytes = _client.CallMethod(15, "", retSize); //out

            return BitConverter.ToUInt32(bytes, 4);
         */

        return 0;
    }

    public DniproQuery Take(int count) {
        _client.AddPacket(17, "", count, 0);

        return this;
    }

    public DniproQuery Skip(int count) {
        _client.AddPacket(18, "", count, 0);

        return this;
    }

    public DniproQuery Sort(String json, boolean isAsc) {
        _client.AddPacket(22, json, isAsc ? (int) 1 : (int) 0, 0);

        return this;
    }

    public int Sum(String json) {
        /*
            int retSize = 0;
            byte[] bytes = _client.CallMethod(19, json, retSize, 0); //out

            return BitConverter.ToUInt32(bytes, 4);
         */

        return 0;
    }

    public DniproQuery Distinct(String json) {
        _client.AddPacket(20, json, 0, 0);

        return this;
    }

    public DniproQuery Join(String json1,
            String json2) {
        /*
            StringBuilder sb = new StringBuilder();
            sb.Append(json1);
            sb.Append(' ');
            sb.Append(json2);
            
            _client.AddPacket(21, sb.ToString(), (uint)json1.Length);
         */

        return this;
    }

    public <T> T[] Select(String json,
            boolean isDistinct) //where T : new()
    {
        /*
            if (json == null)
            {
                json = Serialization.SerializeSchema(typeof(T), 32);
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

                docs[j] = Serialization.DeserializeProxy<T>(docs[j], _client.Encoding.GetString(bytes, i, len));
                
                //string
                i += len;
            }

            return docs;
         */

        return null;
    }

    public <T> T[] Select(T[] docs,
            String json,
            boolean isDistinct) {
        /*
            if (json == null)
            {
                json = Serialization.SerializeSchema(typeof(T), 32);
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

                docs[j] = Serialization.DeserializeProxy<T>(docs[j], _client.Encoding.GetString(bytes, i, len));

                //string
                i += len;
            }

            return docs;
         */

        return null;
    }

    public <T> T Select(T doc,
            String json,
            boolean isDistinct,
            SerializationBehaviour sb) {
        /*
            if (json == null)
            {
                json = Serialization.SerializeSchema(typeof(T), 32);
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

                doc = Serialization.DeserializeProxy<T>(doc, _client.Encoding.GetString(bytes, i, len), sb);

                //string
                i += len;
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

                    if(j != count -1) //not last element
                    {
                        jsons.Append(',');
                    }

                    //string
                    i += len;
                }

                jsons.Append(']');

                doc = Serialization.DeserializeProxy<T>(doc, jsons.ToString(), sb);

            }

            return doc;
         */

        return null;
    }

    public void Drop(String json) {
        _client.CallMethod(23, json, 0, 0);
    }

    public void Update(String json) {
        _client.CallMethod(24, json, 0, 0);
    }

    public <T> void Update(T doc, String json) {
        /*
            if (json == null)
            {
                json = Serialization.SerializeSchema(typeof(T), 32);
            }
            else
            {
                json = Serialization.SerializeProxy<T>(doc, json);
            }

            _client.CallMethod(24, json);
         */
    }

    public void Insert(String json) {
        _client.CallMethod(26, json, 0, 0);
    }

    public <T> void Insert(T doc, String json) {
        /*
            if (json == null)
            {
                json = Serialization.SerializeSchema(typeof(T), 32);
            }
            else
            {
                json = Serialization.SerializeProxy<T>(doc, json);
            }

            _client.CallMethod(26, json);
         */
    }

    public void Print(String json) {
        /*
            PartDoc[] partDocs = Select(json);

            foreach (PartDoc pd in partDocs)
            {
                Console.WriteLine(pd.Json);
            }
         */
    }

    public String Value(String json) {
        /*
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
         */

        return null;
    }

    public DniproQuery GetAll() {
        _client.AddPacket(25, "", 0, 0);

        return this;
    }
}
