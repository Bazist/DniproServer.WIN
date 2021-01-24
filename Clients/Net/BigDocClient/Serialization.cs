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
using System.Reflection;
using System.Collections;
using System.Runtime.Serialization.Json;
using System.IO;
using System.Data;

namespace DniproClient
{
    public class Serialization
    {
        #region Class

        public const uint MaxDepth = 32;

        public class Behaviour
        {
            public virtual Type GetSubType(object parentObject)
            {
                Type subType;

                Type[] types = parentObject.GetType().GetGenericArguments();

                if (types.Length > 0)
                {
                    subType = types[0];
                }
                else
                {
                    subType = typeof(string);
                }

                return subType;
            }

            public virtual object CreateSubItem(object parentObject, Type type)
            {
                return Activator.CreateInstance(type);
            }

            public virtual void AddSubItem(object parentObject, object childObject)
            {
                MethodInfo mi;
                if (childObject != null)
                {
                    mi = parentObject.GetType().GetMethod("Add", new[] { childObject.GetType() });
                }
                else
                {
                    mi = parentObject.GetType().GetMethod("Add");
                }

                mi.Invoke(parentObject,
                          new object[] { childObject });
            }
        }

        public class DataTableBehaviour : Behaviour
        {
            public override Type GetSubType(object parentObject)
            {
                if (parentObject is DataTable)
                {
                    return typeof(DataRow);
                }
                else
                {
                    return base.GetSubType(parentObject);
                }
            }

            public override object CreateSubItem(object parentObject, Type type)
            {
                if (parentObject is DataTable)
                {
                    return ((DataTable)parentObject).NewRow();
                }
                else
                {
                    return base.CreateSubItem(parentObject, type);
                }
            }

            public override void AddSubItem(object parentObject, object childObject)
            {
                if (parentObject is DataTable)
                {
                    ((DataTable)parentObject).Rows.Add((DataRow)childObject);
                }
                else
                {
                    base.AddSubItem(parentObject, childObject);
                }
            }
        }

        #endregion

        #region Schema

        public static string SerializeSchema(Type type,
                                             uint maxDepth = 16)
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
                currDepth > maxDepth ||
                type == typeof(byte[])) //blob
            {
                sb.Append('$');

                return;
            }
            else if (typeof(IEnumerable).IsAssignableFrom(type)) //list array
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

        #region Delegates

        public delegate string AddBlobValueDelegate(byte[] value);

        public delegate byte[] GetBlobValueDelegate(string label);

        #endregion

        #region Changes

        public static string SerializeChanges(object obj,
                                              Change[] changes = null,
                                              bool onlySchema = false,
                                              AddBlobValueDelegate addBlobValue = null)
        {
            if(changes == null)
            {
                changes = new Change[] { Change.Create(obj, ChangeEnum.UpdateRecursively) };
            }

            StringBuilder sb = new StringBuilder();

            bool needSerialize = false;

            SerializeChangesItem(sb, obj, changes, ref needSerialize, false, onlySchema, addBlobValue);

            return sb.ToString();
        }

        private static void SerializeChangesItem(StringBuilder sb,
                                                object obj,
                                                Change[] changes,
                                                ref bool needSerialize,
                                                bool onlyTopLevel,
                                                bool onlySchema,
                                                AddBlobValueDelegate addBlobValue = null)
        {
            if (obj == null)
            {
                needSerialize = false;

                return;
            }

            //primitives
            Type type = obj.GetType();
            if (IsPrimitive(type))
            {
                if (needSerialize)
                {
                    if (!onlySchema)
                    {
                        if (type != typeof(String))
                        {
                            //case insensitive
                            sb.Append('\'').Append(obj.ToString().ToLower()).Append('\'');
                        }
                        else
                        {
                            //case sensitive
                            sb.Append('\'').Append(obj.ToString()).Append('\'');
                        }
                    }
                    else
                    {
                        sb.Append('$');
                    }
                }
            }
            else if(type == typeof(byte[])) //blob
            {
                if (needSerialize && addBlobValue != null)
                {
                    string label = addBlobValue((byte[])obj);
                    
                    sb.Append('\'').Append(label).Append('\'');
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
                        if (change.ChangeType == ChangeEnum.Add)
                        {
                            sb.Append("Add,");
                        }//???
                        //else if (change.ChangeType == ChangeEnum.UpdateRecursively)
                        //{
                        //    sb.Append('@').Append(change.Index.ToString()).Append(",");
                        //}
                        //else if (change.ChangeType == ChangeEnum.UpdateOnlyTopLevel)
                        //{
                        //    sb.Append('@').Append(change.Index.ToString()).Append(",");

                        //    onlyTopLevel = true;
                        //}

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
                            if (change.ChangeType == ChangeEnum.Add)
                            {
                                sb.Append("Add,");
                            }
                            else if (change.ChangeType == ChangeEnum.UpdateRecursively)
                            {
                                sb.Append('@').Append(index.ToString()).Append(',');
                            }
                            else if (change.ChangeType == ChangeEnum.UpdateOnlyTopLevel)
                            {
                                sb.Append('@').Append(index.ToString()).Append(',');
                                
                                //onlyTopLevel = true;
                            }

                            needSerialize = true;
                        }
                    }

                    //serialize item
                    SerializeChangesItem(sb,
                                         en.Current,
                                         changes,
                                         ref needSerialize,
                                         onlyTopLevel,
                                         onlySchema);

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

                if (index > 0)
                {
                    //remove last coma
                    if (sb[sb.Length - 1] == ',')
                    {
                        sb.Remove(sb.Length - 1, 1);
                    }

                    sb.Append(']');

                    needSerialize = needSerializeRoot;
                }
                else
                {
                    needSerialize = false;
                }
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

                    if (val != null)
                    {
                        SerializeChangesItem(sb,
                                             val,
                                             changes,
                                             ref needSerialize,
                                             onlyTopLevel,
                                             onlySchema,
                                             addBlobValue);

                        sb.Append(',');
                    }
                    else
                    {
                        needSerialize = false;
                    }

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

        #region Schema Loads

        public static string SerializeSchemaPart(object obj,
                                                 Type subType)
        {
            StringBuilder sb = new StringBuilder();

            bool needSerialize = false;

            SerializeSchemaPart(sb, obj, subType, ref needSerialize);

            return sb.ToString();
        }

        private static void SerializeSchemaPart(StringBuilder sb,
                                                object obj,
                                                Type subType,
                                                ref bool needSerialize)
        {
            if (obj == null)
                return;

            //primitives
            if (obj.GetType() == subType)
            {
                sb.Append('!');

                needSerialize = true;

                //serialize schema
                sb.Append(SerializeSchema(subType));

                return;
            }
            else if (IsPrimitive(obj.GetType()))
            {
                if (needSerialize)
                {
                    sb.Append('$');
                }
            }
            else if (obj is IEnumerable) //array
            {
                sb.Append('[');

                bool needSerializeRoot = needSerialize;
                bool origNeedSerialize = needSerialize;

                var en = (obj as IEnumerable).GetEnumerator();

                uint index = 0;

                while (en.MoveNext())
                {
                    int idx = sb.Length;

                    //serialize item
                    SerializeSchemaPart(sb,
                                       en.Current,
                                       subType,
                                       ref needSerialize);

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

                if (index > 0)
                {
                    //remove last coma
                    if (sb[sb.Length - 1] == ',')
                    {
                        sb.Remove(sb.Length - 1, 1);
                    }

                    sb.Append(']');

                    needSerialize = needSerializeRoot;
                }
                else
                {
                    needSerialize = false;
                }
            }
            else //object
            {
                //complex
                sb.Append('{');

                bool needSerializeRoot = needSerialize;
                bool origNeedSerialize = needSerialize;

                foreach (var prop in obj.GetType().GetProperties())
                {
                    int idx = sb.Length;

                    sb.Append('\'').Append(prop.Name).Append("\':");

                    //serialize item
                    object val = prop.GetValue(obj, null);

                    if (val != null)
                    {
                        SerializeSchemaPart(sb,
                                            val,
                                            subType,
                                            ref needSerialize);

                        sb.Append(',');
                    }
                    else
                    {
                        needSerialize = false;
                    }

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

                if (index > 0)
                {
                    //remove last coma
                    if (sb[sb.Length - 1] == ',')
                    {
                        sb.Remove(sb.Length - 1, 1);
                    }

                    sb.Append(']');

                    needSerialize = needSerializeRoot;
                }
                else
                {
                    needSerialize = false;
                }
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

                    if (val != null)
                    {
                        SerializeLoadsItem(sb,
                                       val,
                                       loads,
                                       ref needSerialize,
                                       onlyTopLevel);

                        sb.Append(',');
                    }
                    else
                    {
                        needSerialize = false;
                    }

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

        //public static string Serialize(object obj)
        //{
        //    DataContractJsonSerializer js = new DataContractJsonSerializer(obj.GetType());

        //    using (MemoryStream ms = new MemoryStream())
        //    {
        //        using (StreamReader sr = new StreamReader(ms))
        //        {
        //            js.WriteObject(ms, obj);

        //            ms.Position = 0;

        //            return sr.ReadToEnd();
        //        }
        //    }
        //}

        //public static T Deserialize<T>(string json)
        //{
        //    DataContractJsonSerializer deserializer = new DataContractJsonSerializer(typeof(T));

        //    using (MemoryStream stream = new MemoryStream(Encoding.Unicode.GetBytes(json)))
        //    {
        //        return (T)deserializer.ReadObject(stream);
        //    }
        //}

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
            public bool IsBlobValue;
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
                IsBlobValue = false;
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
                                               string jsonObj,
                                               Behaviour sb = null,
                                               AddBlobValueDelegate addBlobValue = null)
        {
            List<string> attrValues = new List<string>();

            if (sb == null)
            {
                sb = new Behaviour();
            }

            DesObj[] objects = new DesObj[32];

            objects[0].Object = obj;

            int level = 0;

            int startIndex = 0;
            int length = 0;

            int i = 0;

            while (i < jsonObj.Length && level >= 0)
            {
                char c = jsonObj[i];

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

                            for (; jsonObj[i] != '"' && jsonObj[i] != '\''; i++)
                            {
                                if(jsonObj[i] == '\\')
                                {
                                    i++;
                                }

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

                            for (; char.IsNumber(jsonObj[i]); i++)
                            {
                                length++;
                            }

                            continue;
                        }
                    case 't': //true
                        {
                            objects[level].IsTrueValue = true;

                            i += 4;

                            continue;
                        }
                    case 'f': //false
                        {
                            objects[level].IsFalseValue = true;

                            i += 5;

                            continue;
                        }
                    case 'n': //null
                        {
                            objects[level].IsNullValue = true;

                            i += 4;

                            continue;
                        }
                    case ':':
                        {
                            string name = jsonObj.Substring(startIndex, length);

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
                                    if (propInfo.PropertyType != typeof(byte[]))
                                    {
                                        val = propInfo.GetValue(objects[level - 1].Object,
                                                                null);
                                    }
                                    else //blob
                                    {
                                        val = addBlobValue((byte[])propInfo.GetValue(objects[level - 1].Object,
                                                           null));
                                    }

                                    objects[level].PropInfo = null;
                                }
                                else if (fieldInfo != null)
                                {
                                    if (fieldInfo.FieldType != typeof(byte[]))
                                    {
                                        val = fieldInfo.GetValue(objects[level - 1].Object);
                                    }
                                    else //blob
                                    {
                                        val = addBlobValue((byte[])fieldInfo.GetValue(objects[level - 1].Object));
                                    }

                                    objects[level].FieldInfo = null;
                                }
                                else
                                {
                                    val = ((DataRow)objects[level - 1].Object)[dataColumn];

                                    objects[level].DataColumn = null;
                                }

                                if (val != null)
                                {
                                    attrValues.Add(val.ToString());
                                }
                                else
                                {
                                    attrValues.Add("null");
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

                i++;
            }

            //create resJson
            int currAttrValue = 0;
            StringBuilder resJson = new StringBuilder();

            foreach(char c in json)
            {
                if (c == '$')
                {
                    resJson.Append('"');

                    resJson.Append(attrValues[currAttrValue++]);

                    resJson.Append('"');
                }
                else
                {
                    resJson.Append(c);
                }
            }

            return resJson.ToString();
        }

        public static string SerializeProxy<T>(T obj,
                                               string json,
                                               Behaviour sb = null,
                                               AddBlobValueDelegate addBlobValue = null)
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
                                if(json[i] == '\\')
                                {
                                    i++;
                                }

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
                                    if (propInfo.PropertyType != typeof(byte[]))
                                    {
                                        val = propInfo.GetValue(objects[level - 1].Object,
                                                                null);

                                    }
                                    else //blob
                                    {
                                        val = addBlobValue((byte[])propInfo.GetValue(objects[level - 1].Object,
                                                           null));
                                    }

                                    objects[level].PropInfo = null;
                                }
                                else if (fieldInfo != null)
                                {
                                    if (fieldInfo.FieldType != typeof(byte[]))
                                    {
                                        val = fieldInfo.GetValue(objects[level - 1].Object);
                                    }
                                    else //blob
                                    {
                                        val = addBlobValue((byte[])fieldInfo.GetValue(objects[level - 1].Object));
                                    }

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

        private static List<string> GetAttrValues(string json)
        {
            List<string> attrValues = new List<string>();

            int level = 0;

            bool hasValue = false;

            int startIndex = 0;
            int length = 0;

            int i = 0;

            while (i < json.Length)
            {
                char c = json[i];

                switch (c)
                {
                    case '[':
                        {
                            break;
                        }
                    case '{':
                        {
                            break;
                        }
                    case '"':
                    case '\'':
                        {
                            startIndex = ++i;
                            length = 0;

                            for (; json[i] != '"' && json[i] != '\''; i++)
                            {
                                if(json[i] == '\\')
                                {
                                    i++;
                                }

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
                            i += 4;

                            hasValue = true;

                            continue;
                        }
                    case 'f': //false
                        {
                            i += 5;

                            hasValue = true;

                            continue;
                        }
                    case 'n': //null
                        {
                            i += 4;

                            hasValue = true;

                            continue;
                        }
                    case ':':
                        {
                            break;
                        }
                    case ']':
                    case '}':
                    case ',':
                        {
                            if (hasValue)
                            {
                                attrValues.Add(json.Substring(startIndex, length));

                                hasValue = false;
                            }

                            if (c == '}')
                            {
                                level--;
                            }

                            if (c == ']')
                            {
                                level--;
                            }

                            break;
                        }
                    default:
                        break;
                }

                i++;
            }

            return attrValues;
        }

        public static T DeserializeProxy<T>(T obj,
                                            string json,
                                            string jsonObj,
                                            Behaviour sb = null,
                                            GetBlobValueDelegate getBlobValue = null)
        {
            List<string> attrValues = GetAttrValues(json);
            int currAttrValue = 0;

            DesObj[] objects = new DesObj[32];

            objects[0].Object = obj;

            int level = 0;

            int startIndex = 0;
            int length = 0;

            int i = 0;

            while (i < jsonObj.Length && level >= 0)
            {
                char c = jsonObj[i];

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

                            for (; jsonObj[i] != '"' && jsonObj[i] != '\''; i++)
                            {
                                if(jsonObj[i] == '\\')
                                {
                                    i++;
                                }

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

                            for (; char.IsNumber(jsonObj[i]); i++)
                            {
                                length++;
                            }

                            continue;
                        }
                    case 't': //true
                        {
                            objects[level].IsTrueValue = true;

                            i += 4;

                            continue;
                        }
                    case 'f': //false
                        {
                            objects[level].IsFalseValue = true;

                            i += 5;

                            continue;
                        }
                    case 'n': //null
                        {
                            objects[level].IsNullValue = true;

                            i += 4;

                            continue;
                        }
                    case ':':
                        {
                            string name = jsonObj.Substring(startIndex, length);

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

                            string val = attrValues[currAttrValue++];

                            if (propInfo != null ||
                                fieldInfo != null ||
                                dataColumn != null) //not primitive
                            {
                                if (propInfo != null)
                                {
                                    propInfo.SetValue(objects[level - 1].Object,
                                                      val, 
                                                      null);

                                    objects[level].PropInfo = null;
                                }
                                else if (fieldInfo != null)
                                {
                                    fieldInfo.SetValue(objects[level - 1].Object,
                                                       val);

                                    objects[level].FieldInfo = null;
                                }
                                else
                                {
                                    //val = ((DataRow)objects[level - 1].Object)[dataColumn];

                                    objects[level].DataColumn = null;
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

                i++;
            }

            return obj;
        }

        public static T DeserializeProxy<T>(T obj,
                                            string json,
                                            Behaviour sb = null,
                                            GetBlobValueDelegate getBlobValue = null)
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

            while (i < json.Length)
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
                                if(json[i] == '\\')
                                {
                                    i++;
                                }

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
                                    object currObj;

                                    if (propInfo.PropertyType != typeof(byte[]))
                                    {
                                        currObj = propInfo.GetValue(objects[level - 1].Object, null);

                                        if (currObj == null)
                                        {
                                            currObj = Activator.CreateInstance(propInfo.PropertyType);
                                            propInfo.SetValue(objects[level - 1].Object, currObj, null);
                                        }
                                    }
                                    else
                                    {
                                        currObj = null;

                                        objects[level].IsBlobValue = true;
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
                                        object currObj;

                                        if (fieldInfo.FieldType != typeof(byte[]))
                                        {
                                            currObj = fieldInfo.GetValue(objects[level - 1].Object);

                                            if (currObj == null)
                                            {
                                                currObj = Activator.CreateInstance(fieldInfo.FieldType);
                                                fieldInfo.SetValue(objects[level - 1].Object, currObj);
                                            }
                                        }
                                        else
                                        {
                                            currObj = null;

                                            objects[level].IsBlobValue = true;
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

                                        if (objects[level].IsBlobValue)
                                        {
                                            if (!string.IsNullOrWhiteSpace(currValueStr))
                                            {
                                                val = getBlobValue(currValueStr);
                                            }
                                            else
                                            {
                                                val = null;
                                            }

                                            objects[level].IsBlobValue = false;
                                        }
                                        else if (propInfo != null)
                                        {
                                            val = ConvertPrimitiveValue(propInfo.PropertyType, currValueStr);
                                        }
                                        else if (fieldInfo != null)
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
                                if (objects[level].Object != null)
                                {
                                    sb.AddSubItem(objects[level - 1].Object, objects[level].Object);

                                    if (objects[level].Type == null) //recreate not primitive instance
                                    {
                                        objects[level].Object = sb.CreateSubItem(objects[level - 1].Object,
                                                                                 objects[level].Object.GetType());
                                    }
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

        public static string Encode(string text)
        {
            return text.Replace("\'", "\\\'").Replace("\"", "\\\"");
        }

        public static string Decode(string text)
        {
            return text.Replace("\\\'", "\'").Replace("\\\"", "\"");
        }

        private static object ConvertPrimitiveValue(Type type, string value)
        {
            if (string.IsNullOrEmpty(value))
            {
                return null;
            }
            else if (type == typeof(string))
            {
                return value;
            }
            else if (type == typeof(int?) || type == typeof(int))
            {
                return Convert.ToInt32(value);
            }
            else if (type == typeof(bool?) || type == typeof(bool))
            {
                return Convert.ToBoolean(value);
            }
            else if (type == typeof(DateTime?) || type == typeof(DateTime))
            {
                return Convert.ToDateTime(value);
            }
            else if (type == typeof(decimal?) || type == typeof(decimal))
            {
                return Convert.ToDecimal(value);
            }
            else if (type == typeof(long?) || type == typeof(long))
            {
                return Convert.ToInt64(value);
            }
            else if (type == typeof(uint?) || type == typeof(uint))
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
                type == typeof(int?) ||
                type == typeof(bool?) ||
                type == typeof(DateTime?) ||
                type == typeof(decimal?) ||
                type == typeof(long?) ||
                type == typeof(uint?) ||
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

        private static void AddSpaces(StringBuilder sb,
                                      int indent)
        {
            for (int i = 0; i < indent; i++)
            {
                sb.Append(' ');
            }
        }

        public static string FormatJson(string json)
        {
            StringBuilder sb = new StringBuilder();

            int indent = 0;

            for (int i = 0; i < json.Length; i++)
            {
                char c = json[i];

                if (c == '\\')
                {
                    sb.Append(json[++i]);

                    continue;
                }

                if (c == '}' || c == ']')
                {
                    indent -= 3;
                    sb.Append("\r\n");
                    AddSpaces(sb, indent);
                }

                sb.Append(c);

                if (c == '{' || c == '[')
                {
                    indent += 3;
                    sb.Append("\r\n");
                    AddSpaces(sb, indent);
                }
                else if (c == ',')
                {
                    sb.Append("\r\n");
                    AddSpaces(sb, indent);
                }
            }

            return sb.ToString();
        }

        #endregion
    }
}
