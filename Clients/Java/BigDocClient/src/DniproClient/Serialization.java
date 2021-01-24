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
public class Serialization
    {
    /*
        public static string SerializeSchema(Type type, uint maxDepth = 3)
        {
            StringBuilder sb = new StringBuilder();

            SerializeSchemaItem(sb, type, 0, maxDepth);

            return sb.ToString();
        }

        private static void SerializeSchemaItem(StringBuilder sb,
                                                Type type,
                                                uint currDepth,
                                                uint maxDepth)
        {
            if (IsPrimitive(type) ||
                currDepth > maxDepth)
            {
                sb.Append('$');

                return;
            }
            else if (type.IsGenericType) //list array
            {
                sb.Append("[R,");

                SerializeSchemaItem(sb,
                                    type.GetGenericArguments()[0],
                                    currDepth + 1,
                                    maxDepth);

                sb.Append(']');
            }
            else
            {
                //complex
                sb.Append('{');

                foreach (var prop in type.GetProperties())
                {
                    sb.Append('"').Append(prop.Name).Append("\":");

                    SerializeSchemaItem(sb,
                                        prop.PropertyType,
                                        currDepth + 1,
                                        maxDepth);

                    sb.Append(",");
                }

                sb.Remove(sb.Length - 1, 1);

                sb.Append('}');
            }
        }

        #endregion

        #region Changes

        public static string SerializeChanges(object obj,
                                              Change[] changes)
        {
            StringBuilder sb = new StringBuilder();

            bool needSerialize = false;

            SerializeChangesItem(sb, obj, changes, ref needSerialize, false);

            return sb.ToString();
        }

        private static void SerializeChangesItem(StringBuilder sb,
                                                object obj,
                                                Change[] changes,
                                                ref bool needSerialize,
                                                bool onlyTopLevel)
        {
            if (obj == null)
                return;

            //primitives
            if (IsPrimitive(obj.GetType()))
            {
                if (needSerialize)
                {
                    sb.Append('\'').Append(obj.ToString()).Append('\'');
                }
            }
            else if (onlyTopLevel)
            {
                needSerialize = false;

                return;
            }
            else if (obj is IEnumerable) //array
            {
                sb.Append('[');

                //update all array items
                if (!needSerialize)
                {
                    var change = changes.FirstOrDefault(c => c.ChildObject.Equals(obj));

                    if (change != null)
                    {
                        if (change.ChangeType == ChangeEnum.AddNewItem)
                        {
                            sb.Append("Add,");
                        }
                        else if (change.ChangeType == ChangeEnum.UpdateRecursively)
                        {
                            sb.Append('@').Append(change.Index.ToString()).Append(",");
                        }
                        else if (change.ChangeType == ChangeEnum.UpdateOnlyTopLevel)
                        {
                            sb.Append('@').Append(change.Index.ToString()).Append(",");

                            onlyTopLevel = true;
                        }

                        needSerialize = true;
                    }
                }

                bool needSerializeRoot = needSerialize;
                bool origNeedSerialize = needSerialize;

                var en = (obj as IEnumerable).GetEnumerator();

                uint index = 0;

                while (en.MoveNext())
                {
                    int idx = sb.Length;

                    //update particular item
                    if (!needSerialize)
                    {
                        var change = changes.FirstOrDefault(c => c.ChildObject.Equals(en.Current));

                        if (change != null)
                        {
                            if (change.ChangeType == ChangeEnum.AddNewItem)
                            {
                                sb.Append("Add,");
                            }
                            else if (change.ChangeType == ChangeEnum.UpdateRecursively)
                            {
                                sb.Append('@').Append(change.Index.ToString()).Append(',');
                            }
                            else if (change.ChangeType == ChangeEnum.UpdateOnlyTopLevel)
                            {
                                sb.Append('@').Append(change.Index.ToString()).Append(',');
                                
                                onlyTopLevel = true;
                            }

                            needSerialize = true;
                        }
                    }

                    //serialize item
                    SerializeChangesItem(sb,
                                         en.Current,
                                         changes,
                                         ref needSerialize,
                                         onlyTopLevel);

                    sb.Append(',');

                    //if we dont need serialize, remove part
                    if (!needSerialize)
                    {
                        sb.Remove(idx, sb.Length - idx);
                    }
                    else
                    {
                        needSerializeRoot = true;
                    }

                    needSerialize = origNeedSerialize;

                    index++;
                }

                //remove last coma
                if (sb[sb.Length - 1] == ',')
                {
                    sb.Remove(sb.Length - 1, 1);
                }

                sb.Append(']');

                needSerialize = needSerializeRoot;
            }
            else //object
            {
                //complex
                sb.Append('{');

                if (!needSerialize) 
                {
                    var change = changes.FirstOrDefault(c => c.ChildObject.Equals(obj));

                    if (change != null)
                    {
                        if (change.ChangeType == ChangeEnum.UpdateOnlyTopLevel)
                        {
                            onlyTopLevel = true;
                        }

                        needSerialize = true;
                    }
                }

                bool needSerializeRoot = needSerialize;
                bool origNeedSerialize = needSerialize;

                foreach (var prop in obj.GetType().GetProperties())
                {
                    int idx = sb.Length;

                    sb.Append('\'').Append(prop.Name).Append("\':");

                    //serialize item
                    object val = prop.GetValue(obj, null);

                    SerializeChangesItem(sb,
                                         val,
                                         changes,
                                         ref needSerialize,
                                         onlyTopLevel);

                    sb.Append(',');

                    //if we dont need serialize, remove part
                    if (!needSerialize)
                    {
                        sb.Remove(idx, sb.Length - idx);
                    }
                    else
                    {
                        needSerializeRoot = true;
                    }

                    needSerialize = origNeedSerialize;
                }

                //remove last coma
                if (sb[sb.Length - 1] == ',')
                {
                    sb.Remove(sb.Length - 1, 1);
                }

                sb.Append('}');

                needSerialize = needSerializeRoot;
            }
        }

        #endregion

        #region Loads

        public static string SerializeLoads(object obj,
                                            Load[] loads)
        {
            StringBuilder sb = new StringBuilder();

            bool needSerialize = false;

            SerializeLoadsItem(sb, obj, loads, ref needSerialize, false);

            return sb.ToString();
        }

        private static void SerializeLoadsItem(StringBuilder sb,
                                                object obj,
                                                Load[] loads,
                                                ref bool needSerialize,
                                                bool onlyTopLevel)
        {
            if (obj == null)
                return;

            //primitives
            if (IsPrimitive(obj.GetType()))
            {
                if (needSerialize)
                {
                    sb.Append('$');
                }
            }
            else if (onlyTopLevel)
            {
                needSerialize = false;

                return;
            }
            else if (obj is IEnumerable) //array
            {
                sb.Append('[');

                //update all array items
                if (!needSerialize)
                {
                    var load = loads.FirstOrDefault(c => c.ChildObject.Equals(obj));

                    if (load != null)
                    {
                        if (load.LoadType == LoadEnum.LoadRecursively)
                        {
                            sb.Append("R,");
                        }
                        else if (load.LoadType == LoadEnum.LoadOnlyTopLevel)
                        {
                            sb.Append("R,");

                            onlyTopLevel = true;
                        }

                        needSerialize = true;
                    }
                }

                bool needSerializeRoot = needSerialize;
                bool origNeedSerialize = needSerialize;

                var en = (obj as IEnumerable).GetEnumerator();

                uint index = 0;

                while (en.MoveNext())
                {
                    int idx = sb.Length;

                    //update particular item
                    if (!needSerialize)
                    {
                        var load = loads.FirstOrDefault(c => c.ChildObject.Equals(en.Current));

                        if (load != null)
                        {
                            if (load.LoadType == LoadEnum.LoadRecursively)
                            {
                                sb.Append('@').Append(load.Index.ToString()).Append(",");
                            }
                            else if (load.LoadType == LoadEnum.LoadOnlyTopLevel)
                            {
                                sb.Append('@').Append(load.Index.ToString()).Append(",");

                                onlyTopLevel = true;
                            }

                            needSerialize = true;
                        }
                    }

                    //serialize item
                    SerializeLoadsItem(sb,
                                       en.Current,
                                       loads,
                                       ref needSerialize,
                                       onlyTopLevel);

                    sb.Append(',');

                    //if we dont need serialize, remove part
                    if (!needSerialize)
                    {
                        sb.Remove(idx, sb.Length - idx);
                    }
                    else
                    {
                        needSerializeRoot = true;
                    }

                    needSerialize = origNeedSerialize;

                    index++;
                }

                //remove last coma
                if (sb[sb.Length - 1] == ',')
                {
                    sb.Remove(sb.Length - 1, 1);
                }

                sb.Append(']');

                needSerialize = needSerializeRoot;
            }
            else //object
            {
                //complex
                sb.Append('{');

                if (!needSerialize)
                {
                    var load = loads.FirstOrDefault(c => c.ChildObject.Equals(obj));

                    if (load != null)
                    {
                        if (load.LoadType == LoadEnum.LoadOnlyTopLevel)
                        {
                            onlyTopLevel = true;
                        }

                        needSerialize = true;
                    }
                }

                bool needSerializeRoot = needSerialize;
                bool origNeedSerialize = needSerialize;

                foreach (var prop in obj.GetType().GetProperties())
                {
                    int idx = sb.Length;

                    sb.Append('\'').Append(prop.Name).Append("\':");

                    //serialize item
                    object val = prop.GetValue(obj, null);

                    SerializeLoadsItem(sb,
                                       val,
                                       loads,
                                       ref needSerialize,
                                       onlyTopLevel);

                    sb.Append(',');

                    //if we dont need serialize, remove part
                    if (!needSerialize)
                    {
                        sb.Remove(idx, sb.Length - idx);
                    }
                    else
                    {
                        needSerializeRoot = true;
                    }

                    needSerialize = origNeedSerialize;
                }

                //remove last coma
                if (sb[sb.Length - 1] == ',')
                {
                    sb.Remove(sb.Length - 1, 1);
                }

                sb.Append('}');

                needSerialize = needSerializeRoot;
            }
        }

        #endregion

        #region Standard

        public static string Serialize(object obj)
        {
            DataContractJsonSerializer js = new DataContractJsonSerializer(obj.GetType());

            using (MemoryStream ms = new MemoryStream())
            {
                using (StreamReader sr = new StreamReader(ms))
                {
                    js.WriteObject(ms, obj);

                    ms.Position = 0;

                    return sr.ReadToEnd();
                }
            }
        }

        public static T Deserialize<T>(string json)
        {
            DataContractJsonSerializer deserializer = new DataContractJsonSerializer(typeof(T));

            using (MemoryStream stream = new MemoryStream(Encoding.Unicode.GetBytes(json)))
            {
                return (T)deserializer.ReadObject(stream);
            }
        }

        #endregion

        #region Proxy

        private struct DesObj
        {
            public object Object;
            public Type Type; //not null for primitives
            public bool IsArray;
            public bool IsTrueValue;
            public bool IsFalseValue;
            public bool IsNullValue;
            public PropertyInfo PropInfo;
            public FieldInfo FieldInfo;
            public DataColumn DataColumn;
            
            public void Reset()
            {
                Object = null;
                Type = null;
                IsArray = false;
                IsTrueValue = false;
                IsFalseValue = false;
                IsNullValue = false;
                PropInfo = null;
                FieldInfo = null;
            }

            public override string ToString()
            {
                if (Object != null)
                {
                    return Object.ToString();
                }
                else if (Type != null)
                {
                    return Type.ToString();
                }
                else
                {
                    return "Undefined";
                }
            }
        }

        public static string SerializeProxy<T>(T obj,
                                               string json,
                                               Behaviour sb = null)
        {
            if (sb == null)
            {
                sb = new Behaviour();
            }

            StringBuilder resJson = new StringBuilder();

            DesObj[] objects = new DesObj[32];

            objects[0].Object = obj;

            int level = 0;

            int startIndex = 0;
            int length = 0;

            int i = 0;

            int idx = json.IndexOf("!{");
            if(idx >= 0)
            {
                resJson.Append(json.Substring(0, idx));

                i = idx + 1;
            }
            
            while (i < json.Length && level >= 0)
            {
                char c = json[i];

                switch (c)
                {
                    case '[':
                        {
                            Type subType = sb.GetSubType(objects[level].Object);

                            if (!IsPrimitive(subType))
                            {
                                object currObj = sb.CreateSubItem(objects[level].Object, subType);

                                objects[++level].Type = null;
                                objects[level].Object = currObj;
                            }
                            else
                            {
                                objects[++level].Type = subType;
                                objects[level].Object = null;
                            }

                            objects[level].IsArray = true;

                            break;
                        }
                    case '{':
                        {
                            objects[++level].IsArray = false;

                            break;
                        }
                    case '"':
                    case '\'':
                        {
                            resJson.Append(json[i]);

                            startIndex = ++i;
                            length = 0;

                            for (; json[i] != '"' && json[i] != '\''; i++)
                            {
                                resJson.Append(json[i]);
                                length++;
                            }

                            break;
                        }
                    case '0':
                    case '1':
                    case '2':
                    case '3':
                    case '4':
                    case '5':
                    case '6':
                    case '7':
                    case '8':
                    case '9':
                        {
                            startIndex = i;
                            length = 0;

                            for (; char.IsNumber(json[i]); i++)
                            {
                                resJson.Append(json[i]);
                                length++;
                            }

                            continue;
                        }
                    case 't': //true
                        {
                            objects[level].IsTrueValue = true;

                            for (int j = 0; j < 4; j++, i++)
                            {
                                resJson.Append(json[i]);
                            }

                            continue;
                        }
                    case 'f': //false
                        {
                            objects[level].IsFalseValue = true;

                            for (int j = 0; j < 5; j++, i++)
                            {
                                resJson.Append(json[i]);
                            }

                            continue;
                        }
                    case 'n': //null
                        {
                            objects[level].IsNullValue = true;

                            for (int j = 0; j < 4; j++, i++)
                            {
                                resJson.Append(json[i]);
                            }

                            continue;
                        }
                    case ':':
                        {
                            string name = json.Substring(startIndex, length);

                            var propInfo = objects[level - 1].Object.GetType().GetProperty(name);

                            if (propInfo != null)
                            {
                                objects[level].PropInfo = propInfo;

                                if (!IsPrimitive(propInfo.PropertyType))
                                {
                                    object currObj = propInfo.GetValue(objects[level - 1].Object, null);

                                    if (currObj == null)
                                    {
                                        currObj = Activator.CreateInstance(propInfo.PropertyType);
                                        propInfo.SetValue(objects[level - 1].Object, currObj, null);
                                    }

                                    objects[level].Object = currObj;
                                }
                            }
                            else
                            {
                                var fieldInfo = objects[level - 1].Object.GetType().GetField(name, BindingFlags.NonPublic | BindingFlags.Public | BindingFlags.Instance);

                                if (fieldInfo != null)
                                {
                                    objects[level].FieldInfo = fieldInfo;

                                    if (!IsPrimitive(fieldInfo.FieldType))
                                    {
                                        object currObj = fieldInfo.GetValue(objects[level - 1].Object);

                                        if (currObj == null)
                                        {
                                            currObj = Activator.CreateInstance(fieldInfo.FieldType);
                                            fieldInfo.SetValue(objects[level - 1].Object, currObj);
                                        }

                                        objects[level].Object = currObj;
                                    }
                                }
                                else
                                {
                                    if (objects[level - 1].Object is DataRow)
                                    {
                                        DataRow dr = (DataRow)objects[level - 1].Object;

                                        objects[level].DataColumn = dr.Table.Columns[name];
                                    }
                                }
                            }

                            break;
                        }
                    case '$':
                        {
                            PropertyInfo propInfo = objects[level].PropInfo;
                            FieldInfo fieldInfo = objects[level].FieldInfo;
                            DataColumn dataColumn = objects[level].DataColumn;

                            object val;

                            if (propInfo != null ||
                                fieldInfo != null ||
                                dataColumn != null) //not primitive
                            {
                                if (propInfo != null)
                                {
                                    val = propInfo.GetValue(objects[level - 1].Object,
                                                            null);

                                    objects[level].PropInfo = null;
                                }
                                else if (fieldInfo != null)
                                {
                                    val = fieldInfo.GetValue(objects[level - 1].Object);

                                    objects[level].FieldInfo = null;
                                }
                                else
                                {
                                    val = ((DataRow)objects[level - 1].Object)[dataColumn];

                                    objects[level].DataColumn = null;
                                }

                                if (val != null)
                                {
                                    resJson.Append("'").Append(val.ToString()).Append("'");
                                }
                                else
                                {
                                    resJson.Append("null");
                                }
                            }

                            i++;
                            
                            continue;
                        }
                    case ']':
                        {
                            objects[level].Reset();

                            level--;

                            break;
                        }
                    case '}':
                        {
                            objects[level].Reset();

                            level--;

                            break;
                        }
                    case ',':
                        {
                            break;
                        }
                    default:
                        break;
                }

                resJson.Append(c);

                i++;
            }

            if (idx >= 0)
            {
                resJson.Append(json.Substring(i));
            }

            return resJson.ToString();
        }

        public static T DeserializeProxy<T>(T obj,
                                            string json,
                                            Behaviour sb = null)
        {
            if (sb == null)
            {
                sb = new Behaviour();
            }

            DesObj[] objects = new DesObj[32];
            
            objects[0].Object = obj;

            int level = 0;

            bool hasValue = false;

            int startIndex = 0;
            int length = 0;

            int i = 0;
            
            while(i < json.Length)
            {
                char c = json[i];

                switch (c)
                {
                    case '[':
                        {
                            Type subType = sb.GetSubType(objects[level].Object);

                            if (!IsPrimitive(subType))
                            {
                                object currObj = sb.CreateSubItem(objects[level].Object, subType);

                                objects[++level].Type = null;
                                objects[level].Object = currObj;
                            }
                            else
                            {
                                objects[++level].Type = subType;
                                objects[level].Object = null;
                            }

                            objects[level].IsArray = true;

                            break;
                        }
                    case '{':
                        {
                            objects[++level].IsArray = false;

                            break;
                        }
                    case '"':
                    case '\'':
                        {
                            startIndex = ++i;
                            length = 0;

                            for (; json[i] != '"' && json[i] != '\''; i++)
                            {
                                length++;
                            }

                            hasValue = true;

                            break;
                        }
                    case '0':
                    case '1':
                    case '2':
                    case '3':
                    case '4':
                    case '5':
                    case '6':
                    case '7':
                    case '8':
                    case '9':
                        {
                            startIndex = i;
                            length = 0;

                            for (; char.IsNumber(json[i]); i++)
                            {
                                length++;
                            }

                            hasValue = true;

                            continue;
                        }
                    case 't': //true
                        {
                            objects[level].IsTrueValue = true;

                            i += 4;

                            hasValue = true;

                            continue;
                        }
                    case 'f': //false
                        {
                            objects[level].IsFalseValue = true;

                            i += 5;

                            hasValue = true;

                            continue;
                        }
                    case 'n': //null
                        {
                            objects[level].IsNullValue = true;

                            i += 4;

                            hasValue = true;

                            continue;
                        }
                    case ':':
                        {
                            string name = json.Substring(startIndex, length);
                            
                            var propInfo = objects[level - 1].Object.GetType().GetProperty(name);

                            if (propInfo != null)
                            {
                                objects[level].PropInfo = propInfo;

                                if (!IsPrimitive(propInfo.PropertyType))
                                {
                                    object currObj = propInfo.GetValue(objects[level - 1].Object, null);

                                    if (currObj == null)
                                    {
                                        currObj = Activator.CreateInstance(propInfo.PropertyType);
                                        propInfo.SetValue(objects[level - 1].Object, currObj, null);
                                    }

                                    objects[level].Object = currObj;
                                }
                            }
                            else
                            {
                                var fieldInfo = objects[level - 1].Object.GetType().GetField(name, BindingFlags.NonPublic | BindingFlags.Public | BindingFlags.Instance);

                                if (fieldInfo != null)
                                {
                                    objects[level].FieldInfo = fieldInfo;

                                    if (!IsPrimitive(fieldInfo.FieldType))
                                    {
                                        object currObj = fieldInfo.GetValue(objects[level - 1].Object);

                                        if (currObj == null)
                                        {
                                            currObj = Activator.CreateInstance(fieldInfo.FieldType);
                                            fieldInfo.SetValue(objects[level - 1].Object, currObj);
                                        }

                                        objects[level].Object = currObj;
                                    }
                                }
                                else
                                {
                                    if (objects[level - 1].Object is DataRow)
                                    {
                                        DataRow dr = (DataRow)objects[level - 1].Object;

                                        objects[level].DataColumn = dr.Table.Columns[name];
                                    }
                                }
                            }
                            
                            break;
                        }
                    case ']':
                    case '}':
                    case ',':
                        {
                            if (hasValue)
                            {
                                PropertyInfo propInfo = objects[level].PropInfo;
                                FieldInfo fieldInfo = objects[level].FieldInfo;
                                DataColumn dataColumn = objects[level].DataColumn;

                                if (propInfo != null ||
                                    fieldInfo != null ||
                                    dataColumn != null) //not primitive
                                {
                                    object val = null;

                                    if(objects[level].IsTrueValue)
                                    {
                                        objects[level].IsTrueValue = false;

                                        val = true;
                                    }
                                    else if(objects[level].IsFalseValue)
                                    {
                                        objects[level].IsFalseValue = false;

                                        val = false;
                                    }
                                    else if(objects[level].IsNullValue)
                                    {
                                        objects[level].IsNullValue = false;

                                        val = null;
                                    }
                                    else
                                    {
                                        string currValueStr = json.Substring(startIndex, length);

                                        if (propInfo != null)
                                        {
                                            val = ConvertPrimitiveValue(propInfo.PropertyType, currValueStr);
                                        }
                                        else if(fieldInfo != null)
                                        {
                                            val = ConvertPrimitiveValue(fieldInfo.FieldType, currValueStr);
                                        }
                                        else //if (dataColumn != null)
                                        {
                                            val = ConvertPrimitiveValue(dataColumn.DataType, currValueStr);
                                        }
                                    }

                                    if (propInfo != null)
                                    {
                                        if (propInfo.CanWrite)
                                        {
                                            propInfo.SetValue(objects[level - 1].Object,
                                                              val,
                                                              null);
                                        }

                                        objects[level].PropInfo = null;
                                    }
                                    else if(fieldInfo != null)
                                    {
                                        fieldInfo.SetValue(objects[level - 1].Object,
                                                           val);

                                        objects[level].FieldInfo = null;
                                    }
                                    else //if (dataColumn != null)
                                    {
                                        ((DataRow)objects[level - 1].Object)[dataColumn] = val;

                                        objects[level].DataColumn = null;
                                    }
                                }
                                else //primitive
                                {
                                    object val;

                                    if(objects[level].IsTrueValue)
                                    {
                                        objects[level].IsTrueValue = false;

                                        val = true;
                                    }
                                    else if(objects[level].IsFalseValue)
                                    {
                                        objects[level].IsFalseValue = false;

                                        val = false;
                                    }
                                    else if (objects[level].IsNullValue)
                                    {
                                        objects[level].IsNullValue = false;

                                        val = null;
                                    }
                                    else
                                    {
                                        string currValueStr = json.Substring(startIndex, length);

                                        val = ConvertPrimitiveValue(objects[level].Type, currValueStr);
                                    }

                                    objects[level].Object = val;
                                }

                                hasValue = false;
                            }

                            if (c == '}')
                            {
                                objects[level].Reset();

                                level--;
                            }
                            
                            if ((c == ',' || c == ']') && objects[level].IsArray)
                            {
                                sb.AddSubItem(objects[level - 1].Object, objects[level].Object);

                                if (objects[level].Type == null) //recreate not primitive instance
                                {
                                    objects[level].Object = sb.CreateSubItem(objects[level - 1].Object, 
                                                                             objects[level].Object.GetType());
                                }
                            }

                            if (c == ']')
                            {
                                objects[level].Reset();

                                level--;
                            }

                            break;
                        }
                    default:
                        break;
                }

                i++;
            }

            return obj;
        }

        #endregion

        #region Helpers

        private static object ConvertPrimitiveValue(Type type, string value)
        {
            if (type == typeof(string))
            {
                return value;
            }
            else if (type == typeof(int))
            {
                if (!string.IsNullOrEmpty(value))
                {
                    return Convert.ToInt32(value);
                }

                return null;
            }
            else if (type == typeof(bool))
            {
                return Convert.ToBoolean(value);
            }
            else if (type == typeof(DateTime))
            {
                return Convert.ToDateTime(value);
            }
            else if (type == typeof(decimal))
            {
                return Convert.ToDecimal(value);
            }
            else if (type == typeof(long))
            {
                return Convert.ToInt64(value);
            }
            else if (type == typeof(uint))
            {
                return Convert.ToUInt32(value);
            }
            else
            {
                return null;
            }
        }

        private static bool IsPrimitive(Type type)
        {
            if (type == typeof(string) ||
                type == typeof(int) ||
                type == typeof(bool) ||
                type == typeof(DateTime) ||
                type == typeof(decimal) ||
                type == typeof(long) ||
                type == typeof(uint))
            {
                return true;
            }
            else
            {
                return false;
            }
        }

        public static void ValidateJson(string json)
        {
            int bracket1 = 0;
            int bracket2 = 0;
            int quotes = 0;

            for (int i = 0; i < json.Length; i++)
            {
                switch (json[i])
                {
                    case '{':
                        bracket1++;
                        break;
                    case '}':
                        bracket1--;
                        break;
                    case '[':
                        bracket2++;
                        break;
                    case ']':
                        bracket2++;
                        break;
                    case '"':
                    case '\'':
                        {
                            quotes++;

                            for (++i; i < json.Length; i++)
                            {
                                if (json[i] == '"' || json[i] == '\'')
                                {
                                    quotes--;

                                    break;
                                }
                            }

                            break;
                        }
                    default:
                        break;
                };
            }

            if (bracket1 != 0)
            {
                throw new Exception("Brackets '{' and '}' don't match to each other.");
            }

            if (bracket2 != 0)
            {
                throw new Exception("Brackets '[' and ']' don't match to each other.");
            }

            if (quotes != 0)
            {
                throw new Exception("Quotes don't match to each other.");
            }
        }
*/
    }
