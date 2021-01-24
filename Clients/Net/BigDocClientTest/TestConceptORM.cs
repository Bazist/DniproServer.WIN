using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;

namespace DniproClientTest
{
    #region Attr

    public enum AttrType
    {
        None,
        Value,
        Extract,
        ExtractTopLevel,
        Range,
        LessThan,
        LessOrEqualThan,
        GreaterThan,
        GreaterOrEqualThan,
        Pattern
    }

    public class Attr<T>
    {
        public Attr(T value)
        {
            Type = AttrType.Value;
            Value = value;
        }

        private Attr(AttrType type)
        {
            Type = type;
        }

        private Attr(AttrType type, T value)
        {
            Type = type;

            Value = value;
        }

        private Attr(AttrType type, T from, T to)
        {
            Type = type;
            From = from;
            To = to;
        }

        private AttrType Type { get; }

        public T From { get; }
        public T To { get; }

        public T Value { get; }

        public static Attr<T> Extract { get; } = new Attr<T>(AttrType.Extract);
        public static Attr<T> ExtractTopLevel { get; } = new Attr<T>(AttrType.ExtractTopLevel);
        public static Attr<T> Range(T from, T to) => new Attr<T>(AttrType.Range, from, to);
        public static Attr<T> LessThan(T value) => new Attr<T>(AttrType.LessThan, value);
        public static Attr<T> LessOrEqualThan(T value) => new Attr<T>(AttrType.LessOrEqualThan, value);
        public static Attr<T> GreaterThan(T value) => new Attr<T>(AttrType.GreaterThan, value);
        public static Attr<T> GreaterOrEqualThan(T value) => new Attr<T>(AttrType.GreaterOrEqualThan, value);
        public static Attr<string> Pattern(string template) => new Attr<string>(AttrType.Pattern, template);
    }

    #endregion

    #region AttrList

    public enum AttrListType
    {
        None,
        Value,
        Extract,
        ExtractTopLevel,
        Portion,
        Last,
        Add,
        In,
        NotIn,
        Any,
        Repeat //???
    }

    public class AttrList<T>
    {
        public AttrList(T[] value)
        {
            Type = AttrListType.Value;
            Value = value;
        }

        private AttrList(AttrListType type, int position, int count)
        {
            Type = type;
            Position = position;
            Count = count;
        }

        private AttrList(AttrListType type, T[] value)
        {
            Type = type;
            Value = value;
        }

        public AttrListType Type { get; set; }

        public int Position { get; }

        public int Count { get; }

        public T[] Value { get; }

        public static AttrList<T> Portion(int count) => new AttrList<T>(AttrListType.Portion, 0, count);

        public static AttrList<T> Portion(int position, int count) => new AttrList<T>(AttrListType.Portion, position, count);

        public static AttrList<T> Last(int count) => new AttrList<T>(AttrListType.Last, 0, count);

        public static AttrList<T> Add(params T[] value) => new AttrList<T>(AttrListType.Add, value);

        public static AttrList<T> In(params T[] value) => new AttrList<T>(AttrListType.In, value);

        public static AttrList<T> NotIn(params T[] value) => new AttrList<T>(AttrListType.NotIn, value);

        public static AttrList<T> Any(params T[] value) => new AttrList<T>(AttrListType.Any, value);
    }

    #endregion

    public class DocumentItem
    {
        public Attr<string> Name;
    }

    public class Document
    {
        public Attr<int> ID;

        public Attr<DocumentItem> Item;

        public AttrList<DocumentItem> Items;

    }

    public class TestQuery
    {
        public int AddDoc<T>(T doc)
        {
            throw new NotImplementedException();
        }

        public int GetPartDoc<T>(T doc, T extractAttrs, int docID)
        {
            throw new NotImplementedException();
        }

        public int GetWhere<T>(T doc)
        {
            throw new NotImplementedException();
        }

        public void Test()
        {
            //add doc
            AddDoc(new Document
            {
                ID = new Attr<int>(5),
                Items = new AttrList<DocumentItem>(new[] { new DocumentItem() })
            });

            //extract part doc
            Document doc = null;

            GetPartDoc(doc,
                       new Document
                       {
                           ID = Attr<int>.Extract,
                           Item = Attr<DocumentItem>.ExtractTopLevel
                       },
                       1);

            //get where
            GetWhere(new Document
            {
                ID = Attr<int>.Range(1, 5)
            });

            //get part array
            GetPartDoc(doc,
                       new Document
                       {
                           Items = AttrList<DocumentItem>.Portion(5)
                       },
                       5);

            //get where
            GetWhere(new Document
            {
                Items = AttrList<DocumentItem>.In(new DocumentItem { Name = Attr<string>.Pattern("Hello*") })
            });

        }
    }
}
