﻿//------------------------------------------------------------------------------
// <auto-generated>
//     This code was generated by a tool.
//     Runtime Version:4.0.30319.42000
//
//     Changes to this file may cause incorrect behavior and will be lost if
//     the code is regenerated.
// </auto-generated>
//------------------------------------------------------------------------------

namespace Booben.FTSearchService7 {
    using System.Runtime.Serialization;
    using System;
    
    
    [System.Diagnostics.DebuggerStepThroughAttribute()]
    [System.CodeDom.Compiler.GeneratedCodeAttribute("System.Runtime.Serialization", "4.0.0.0")]
    [System.Runtime.Serialization.DataContractAttribute(Name="FTSearch.RelevantResult", Namespace="http://schemas.datacontract.org/2004/07/FTSearchWCF")]
    [System.SerializableAttribute()]
    public partial class FTSearchRelevantResult : object, System.Runtime.Serialization.IExtensibleDataObject, System.ComponentModel.INotifyPropertyChanged {
        
        [System.NonSerializedAttribute()]
        private System.Runtime.Serialization.ExtensionDataObject extensionDataField;
        
        [System.Runtime.Serialization.OptionalFieldAttribute()]
        private string MatchWordsField;
        
        [System.Runtime.Serialization.OptionalFieldAttribute()]
        private string[] ResultsField;
        
        [global::System.ComponentModel.BrowsableAttribute(false)]
        public System.Runtime.Serialization.ExtensionDataObject ExtensionData {
            get {
                return this.extensionDataField;
            }
            set {
                this.extensionDataField = value;
            }
        }
        
        [System.Runtime.Serialization.DataMemberAttribute()]
        public string MatchWords {
            get {
                return this.MatchWordsField;
            }
            set {
                if ((object.ReferenceEquals(this.MatchWordsField, value) != true)) {
                    this.MatchWordsField = value;
                    this.RaisePropertyChanged("MatchWords");
                }
            }
        }
        
        [System.Runtime.Serialization.DataMemberAttribute()]
        public string[] Results {
            get {
                return this.ResultsField;
            }
            set {
                if ((object.ReferenceEquals(this.ResultsField, value) != true)) {
                    this.ResultsField = value;
                    this.RaisePropertyChanged("Results");
                }
            }
        }
        
        public event System.ComponentModel.PropertyChangedEventHandler PropertyChanged;
        
        protected void RaisePropertyChanged(string propertyName) {
            System.ComponentModel.PropertyChangedEventHandler propertyChanged = this.PropertyChanged;
            if ((propertyChanged != null)) {
                propertyChanged(this, new System.ComponentModel.PropertyChangedEventArgs(propertyName));
            }
        }
    }
    
    [System.Diagnostics.DebuggerStepThroughAttribute()]
    [System.CodeDom.Compiler.GeneratedCodeAttribute("System.Runtime.Serialization", "4.0.0.0")]
    [System.Runtime.Serialization.DataContractAttribute(Name="FTSearch.Selector", Namespace="http://schemas.datacontract.org/2004/07/FTSearchWCF")]
    [System.SerializableAttribute()]
    public partial class FTSearchSelector : object, System.Runtime.Serialization.IExtensibleDataObject, System.ComponentModel.INotifyPropertyChanged {
        
        [System.NonSerializedAttribute()]
        private System.Runtime.Serialization.ExtensionDataObject extensionDataField;
        
        [System.Runtime.Serialization.OptionalFieldAttribute()]
        private byte AgregationSortTypeField;
        
        [System.Runtime.Serialization.OptionalFieldAttribute()]
        private byte AgregationTypeField;
        
        [System.Runtime.Serialization.OptionalFieldAttribute()]
        private byte ConditionTypeField;
        
        [System.Runtime.Serialization.OptionalFieldAttribute()]
        private string FilePathField;
        
        [System.Runtime.Serialization.OptionalFieldAttribute()]
        private bool IsSortAscField;
        
        [System.Runtime.Serialization.OptionalFieldAttribute()]
        private string MaxBoundField;
        
        [System.Runtime.Serialization.OptionalFieldAttribute()]
        private string MinBoundField;
        
        [System.Runtime.Serialization.OptionalFieldAttribute()]
        private string NameField;
        
        [System.Runtime.Serialization.OptionalFieldAttribute()]
        private uint Operand2Field;
        
        [System.Runtime.Serialization.OptionalFieldAttribute()]
        private byte RangeTypeField;
        
        [System.Runtime.Serialization.OptionalFieldAttribute()]
        private byte SelectorTypeField;
        
        [global::System.ComponentModel.BrowsableAttribute(false)]
        public System.Runtime.Serialization.ExtensionDataObject ExtensionData {
            get {
                return this.extensionDataField;
            }
            set {
                this.extensionDataField = value;
            }
        }
        
        [System.Runtime.Serialization.DataMemberAttribute()]
        public byte AgregationSortType {
            get {
                return this.AgregationSortTypeField;
            }
            set {
                if ((this.AgregationSortTypeField.Equals(value) != true)) {
                    this.AgregationSortTypeField = value;
                    this.RaisePropertyChanged("AgregationSortType");
                }
            }
        }
        
        [System.Runtime.Serialization.DataMemberAttribute()]
        public byte AgregationType {
            get {
                return this.AgregationTypeField;
            }
            set {
                if ((this.AgregationTypeField.Equals(value) != true)) {
                    this.AgregationTypeField = value;
                    this.RaisePropertyChanged("AgregationType");
                }
            }
        }
        
        [System.Runtime.Serialization.DataMemberAttribute()]
        public byte ConditionType {
            get {
                return this.ConditionTypeField;
            }
            set {
                if ((this.ConditionTypeField.Equals(value) != true)) {
                    this.ConditionTypeField = value;
                    this.RaisePropertyChanged("ConditionType");
                }
            }
        }
        
        [System.Runtime.Serialization.DataMemberAttribute()]
        public string FilePath {
            get {
                return this.FilePathField;
            }
            set {
                if ((object.ReferenceEquals(this.FilePathField, value) != true)) {
                    this.FilePathField = value;
                    this.RaisePropertyChanged("FilePath");
                }
            }
        }
        
        [System.Runtime.Serialization.DataMemberAttribute()]
        public bool IsSortAsc {
            get {
                return this.IsSortAscField;
            }
            set {
                if ((this.IsSortAscField.Equals(value) != true)) {
                    this.IsSortAscField = value;
                    this.RaisePropertyChanged("IsSortAsc");
                }
            }
        }
        
        [System.Runtime.Serialization.DataMemberAttribute()]
        public string MaxBound {
            get {
                return this.MaxBoundField;
            }
            set {
                if ((object.ReferenceEquals(this.MaxBoundField, value) != true)) {
                    this.MaxBoundField = value;
                    this.RaisePropertyChanged("MaxBound");
                }
            }
        }
        
        [System.Runtime.Serialization.DataMemberAttribute()]
        public string MinBound {
            get {
                return this.MinBoundField;
            }
            set {
                if ((object.ReferenceEquals(this.MinBoundField, value) != true)) {
                    this.MinBoundField = value;
                    this.RaisePropertyChanged("MinBound");
                }
            }
        }
        
        [System.Runtime.Serialization.DataMemberAttribute()]
        public string Name {
            get {
                return this.NameField;
            }
            set {
                if ((object.ReferenceEquals(this.NameField, value) != true)) {
                    this.NameField = value;
                    this.RaisePropertyChanged("Name");
                }
            }
        }
        
        [System.Runtime.Serialization.DataMemberAttribute()]
        public uint Operand2 {
            get {
                return this.Operand2Field;
            }
            set {
                if ((this.Operand2Field.Equals(value) != true)) {
                    this.Operand2Field = value;
                    this.RaisePropertyChanged("Operand2");
                }
            }
        }
        
        [System.Runtime.Serialization.DataMemberAttribute()]
        public byte RangeType {
            get {
                return this.RangeTypeField;
            }
            set {
                if ((this.RangeTypeField.Equals(value) != true)) {
                    this.RangeTypeField = value;
                    this.RaisePropertyChanged("RangeType");
                }
            }
        }
        
        [System.Runtime.Serialization.DataMemberAttribute()]
        public byte SelectorType {
            get {
                return this.SelectorTypeField;
            }
            set {
                if ((this.SelectorTypeField.Equals(value) != true)) {
                    this.SelectorTypeField = value;
                    this.RaisePropertyChanged("SelectorType");
                }
            }
        }
        
        public event System.ComponentModel.PropertyChangedEventHandler PropertyChanged;
        
        protected void RaisePropertyChanged(string propertyName) {
            System.ComponentModel.PropertyChangedEventHandler propertyChanged = this.PropertyChanged;
            if ((propertyChanged != null)) {
                propertyChanged(this, new System.ComponentModel.PropertyChangedEventArgs(propertyName));
            }
        }
    }
    
    [System.Diagnostics.DebuggerStepThroughAttribute()]
    [System.CodeDom.Compiler.GeneratedCodeAttribute("System.Runtime.Serialization", "4.0.0.0")]
    [System.Runtime.Serialization.DataContractAttribute(Name="FTSearch.FTSInstanceInfo", Namespace="http://schemas.datacontract.org/2004/07/FTSearchWCF")]
    [System.SerializableAttribute()]
    public partial struct FTSearchFTSInstanceInfo : System.Runtime.Serialization.IExtensibleDataObject, System.ComponentModel.INotifyPropertyChanged {
        
        [System.NonSerializedAttribute()]
        private System.Runtime.Serialization.ExtensionDataObject extensionDataField;
        
        [System.Runtime.Serialization.OptionalFieldAttribute()]
        private uint ControlValueField;
        
        [System.Runtime.Serialization.OptionalFieldAttribute()]
        private uint CountWordsHDDField;
        
        [System.Runtime.Serialization.OptionalFieldAttribute()]
        private uint CountWordsRAMField;
        
        [System.Runtime.Serialization.OptionalFieldAttribute()]
        private uint DocumentNameSizeField;
        
        [System.Runtime.Serialization.OptionalFieldAttribute()]
        private bool HasErrorField;
        
        [System.Runtime.Serialization.OptionalFieldAttribute()]
        private string LastErrorMessageField;
        
        [System.Runtime.Serialization.OptionalFieldAttribute()]
        private uint LastNameIDHDDField;
        
        [System.Runtime.Serialization.OptionalFieldAttribute()]
        private uint LastNameIDRAMField;
        
        [System.Runtime.Serialization.OptionalFieldAttribute()]
        private uint RelevantLevelField;
        
        [System.Runtime.Serialization.OptionalFieldAttribute()]
        private uint ReservedValue1Field;
        
        [System.Runtime.Serialization.OptionalFieldAttribute()]
        private uint ReservedValue2Field;
        
        [System.Runtime.Serialization.OptionalFieldAttribute()]
        private uint ReservedValue3Field;
        
        [System.Runtime.Serialization.OptionalFieldAttribute()]
        private uint ReservedValue4Field;
        
        [System.Runtime.Serialization.OptionalFieldAttribute()]
        private uint ReservedValue5Field;
        
        [System.Runtime.Serialization.OptionalFieldAttribute()]
        private uint TotalMemoryField;
        
        [System.Runtime.Serialization.OptionalFieldAttribute()]
        private uint UsedMemoryField;
        
        [System.Runtime.Serialization.OptionalFieldAttribute()]
        private uint VersionField;
        
        [System.Runtime.Serialization.OptionalFieldAttribute()]
        private uint WordsHeaderBaseField;
        
        public System.Runtime.Serialization.ExtensionDataObject ExtensionData {
            get {
                return this.extensionDataField;
            }
            set {
                this.extensionDataField = value;
            }
        }
        
        [System.Runtime.Serialization.DataMemberAttribute()]
        public uint ControlValue {
            get {
                return this.ControlValueField;
            }
            set {
                if ((this.ControlValueField.Equals(value) != true)) {
                    this.ControlValueField = value;
                    this.RaisePropertyChanged("ControlValue");
                }
            }
        }
        
        [System.Runtime.Serialization.DataMemberAttribute()]
        public uint CountWordsHDD {
            get {
                return this.CountWordsHDDField;
            }
            set {
                if ((this.CountWordsHDDField.Equals(value) != true)) {
                    this.CountWordsHDDField = value;
                    this.RaisePropertyChanged("CountWordsHDD");
                }
            }
        }
        
        [System.Runtime.Serialization.DataMemberAttribute()]
        public uint CountWordsRAM {
            get {
                return this.CountWordsRAMField;
            }
            set {
                if ((this.CountWordsRAMField.Equals(value) != true)) {
                    this.CountWordsRAMField = value;
                    this.RaisePropertyChanged("CountWordsRAM");
                }
            }
        }
        
        [System.Runtime.Serialization.DataMemberAttribute()]
        public uint DocumentNameSize {
            get {
                return this.DocumentNameSizeField;
            }
            set {
                if ((this.DocumentNameSizeField.Equals(value) != true)) {
                    this.DocumentNameSizeField = value;
                    this.RaisePropertyChanged("DocumentNameSize");
                }
            }
        }
        
        [System.Runtime.Serialization.DataMemberAttribute()]
        public bool HasError {
            get {
                return this.HasErrorField;
            }
            set {
                if ((this.HasErrorField.Equals(value) != true)) {
                    this.HasErrorField = value;
                    this.RaisePropertyChanged("HasError");
                }
            }
        }
        
        [System.Runtime.Serialization.DataMemberAttribute()]
        public string LastErrorMessage {
            get {
                return this.LastErrorMessageField;
            }
            set {
                if ((object.ReferenceEquals(this.LastErrorMessageField, value) != true)) {
                    this.LastErrorMessageField = value;
                    this.RaisePropertyChanged("LastErrorMessage");
                }
            }
        }
        
        [System.Runtime.Serialization.DataMemberAttribute()]
        public uint LastNameIDHDD {
            get {
                return this.LastNameIDHDDField;
            }
            set {
                if ((this.LastNameIDHDDField.Equals(value) != true)) {
                    this.LastNameIDHDDField = value;
                    this.RaisePropertyChanged("LastNameIDHDD");
                }
            }
        }
        
        [System.Runtime.Serialization.DataMemberAttribute()]
        public uint LastNameIDRAM {
            get {
                return this.LastNameIDRAMField;
            }
            set {
                if ((this.LastNameIDRAMField.Equals(value) != true)) {
                    this.LastNameIDRAMField = value;
                    this.RaisePropertyChanged("LastNameIDRAM");
                }
            }
        }
        
        [System.Runtime.Serialization.DataMemberAttribute()]
        public uint RelevantLevel {
            get {
                return this.RelevantLevelField;
            }
            set {
                if ((this.RelevantLevelField.Equals(value) != true)) {
                    this.RelevantLevelField = value;
                    this.RaisePropertyChanged("RelevantLevel");
                }
            }
        }
        
        [System.Runtime.Serialization.DataMemberAttribute()]
        public uint ReservedValue1 {
            get {
                return this.ReservedValue1Field;
            }
            set {
                if ((this.ReservedValue1Field.Equals(value) != true)) {
                    this.ReservedValue1Field = value;
                    this.RaisePropertyChanged("ReservedValue1");
                }
            }
        }
        
        [System.Runtime.Serialization.DataMemberAttribute()]
        public uint ReservedValue2 {
            get {
                return this.ReservedValue2Field;
            }
            set {
                if ((this.ReservedValue2Field.Equals(value) != true)) {
                    this.ReservedValue2Field = value;
                    this.RaisePropertyChanged("ReservedValue2");
                }
            }
        }
        
        [System.Runtime.Serialization.DataMemberAttribute()]
        public uint ReservedValue3 {
            get {
                return this.ReservedValue3Field;
            }
            set {
                if ((this.ReservedValue3Field.Equals(value) != true)) {
                    this.ReservedValue3Field = value;
                    this.RaisePropertyChanged("ReservedValue3");
                }
            }
        }
        
        [System.Runtime.Serialization.DataMemberAttribute()]
        public uint ReservedValue4 {
            get {
                return this.ReservedValue4Field;
            }
            set {
                if ((this.ReservedValue4Field.Equals(value) != true)) {
                    this.ReservedValue4Field = value;
                    this.RaisePropertyChanged("ReservedValue4");
                }
            }
        }
        
        [System.Runtime.Serialization.DataMemberAttribute()]
        public uint ReservedValue5 {
            get {
                return this.ReservedValue5Field;
            }
            set {
                if ((this.ReservedValue5Field.Equals(value) != true)) {
                    this.ReservedValue5Field = value;
                    this.RaisePropertyChanged("ReservedValue5");
                }
            }
        }
        
        [System.Runtime.Serialization.DataMemberAttribute()]
        public uint TotalMemory {
            get {
                return this.TotalMemoryField;
            }
            set {
                if ((this.TotalMemoryField.Equals(value) != true)) {
                    this.TotalMemoryField = value;
                    this.RaisePropertyChanged("TotalMemory");
                }
            }
        }
        
        [System.Runtime.Serialization.DataMemberAttribute()]
        public uint UsedMemory {
            get {
                return this.UsedMemoryField;
            }
            set {
                if ((this.UsedMemoryField.Equals(value) != true)) {
                    this.UsedMemoryField = value;
                    this.RaisePropertyChanged("UsedMemory");
                }
            }
        }
        
        [System.Runtime.Serialization.DataMemberAttribute()]
        public uint Version {
            get {
                return this.VersionField;
            }
            set {
                if ((this.VersionField.Equals(value) != true)) {
                    this.VersionField = value;
                    this.RaisePropertyChanged("Version");
                }
            }
        }
        
        [System.Runtime.Serialization.DataMemberAttribute()]
        public uint WordsHeaderBase {
            get {
                return this.WordsHeaderBaseField;
            }
            set {
                if ((this.WordsHeaderBaseField.Equals(value) != true)) {
                    this.WordsHeaderBaseField = value;
                    this.RaisePropertyChanged("WordsHeaderBase");
                }
            }
        }
        
        public event System.ComponentModel.PropertyChangedEventHandler PropertyChanged;
        
        void RaisePropertyChanged(string propertyName) {
            System.ComponentModel.PropertyChangedEventHandler propertyChanged = this.PropertyChanged;
            if ((propertyChanged != null)) {
                propertyChanged(this, new System.ComponentModel.PropertyChangedEventArgs(propertyName));
            }
        }
    }
    
    [System.CodeDom.Compiler.GeneratedCodeAttribute("System.ServiceModel", "4.0.0.0")]
    [System.ServiceModel.ServiceContractAttribute(ConfigurationName="FTSearchService7.FTSearchService")]
    public interface FTSearchService {
        
        [System.ServiceModel.OperationContractAttribute(Action="http://tempuri.org/FTSearchService/StemContent", ReplyAction="http://tempuri.org/FTSearchService/StemContentResponse")]
        string StemContent(string contentText);
        
        [System.ServiceModel.OperationContractAttribute(Action="http://tempuri.org/FTSearchService/IndexText", ReplyAction="http://tempuri.org/FTSearchService/IndexTextResponse")]
        void IndexText(string aliasName, string contentText);
        
        [System.ServiceModel.OperationContractAttribute(Action="http://tempuri.org/FTSearchService/SearchPhrase", ReplyAction="http://tempuri.org/FTSearchService/SearchPhraseResponse")]
        Booben.FTSearchService7.FTSearchRelevantResult SearchPhrase(string phrase, uint minPage, uint maxPage);
        
        [System.ServiceModel.OperationContractAttribute(Action="http://tempuri.org/FTSearchService/SearchPhraseRel", ReplyAction="http://tempuri.org/FTSearchService/SearchPhraseRelResponse")]
        Booben.FTSearchService7.FTSearchRelevantResult SearchPhraseRel(string phrase, uint minPage, uint maxPage);
        
        [System.ServiceModel.OperationContractAttribute(Action="http://tempuri.org/FTSearchService/SearchQuery", ReplyAction="http://tempuri.org/FTSearchService/SearchQueryResponse")]
        string[] SearchQuery(Booben.FTSearchService7.FTSearchSelector[] selectors, uint minPage, uint maxPage, uint skip, bool agregateBySubject);
        
        [System.ServiceModel.OperationContractAttribute(Action="http://tempuri.org/FTSearchService/SearchNewMems", ReplyAction="http://tempuri.org/FTSearchService/SearchNewMemsResponse")]
        Booben.FTSearchService7.FTSearchRelevantResult SearchNewMems(System.DateTime startDate1, System.DateTime startDate2, System.DateTime endDate2);
        
        [System.ServiceModel.OperationContractAttribute(Action="http://tempuri.org/FTSearchService/CalculateTrend", ReplyAction="http://tempuri.org/FTSearchService/CalculateTrendResponse")]
        string CalculateTrend(string phrase, uint count, uint minPage, uint maxPage);
        
        [System.ServiceModel.OperationContractAttribute(Action="http://tempuri.org/FTSearchService/GetInfo", ReplyAction="http://tempuri.org/FTSearchService/GetInfoResponse")]
        Booben.FTSearchService7.FTSearchFTSInstanceInfo GetInfo();
        
        [System.ServiceModel.OperationContractAttribute(Action="http://tempuri.org/FTSearchService/SearchHtmlSeldomWords", ReplyAction="http://tempuri.org/FTSearchService/SearchHtmlSeldomWordsResponse")]
        Booben.FTSearchService7.FTSearchRelevantResult SearchHtmlSeldomWords(string text);
        
        [System.ServiceModel.OperationContractAttribute(Action="http://tempuri.org/FTSearchService/SaveIndex", ReplyAction="http://tempuri.org/FTSearchService/SaveIndexResponse")]
        void SaveIndex();
    }
    
    [System.CodeDom.Compiler.GeneratedCodeAttribute("System.ServiceModel", "4.0.0.0")]
    public interface FTSearchServiceChannel : Booben.FTSearchService7.FTSearchService, System.ServiceModel.IClientChannel {
    }
    
    [System.Diagnostics.DebuggerStepThroughAttribute()]
    [System.CodeDom.Compiler.GeneratedCodeAttribute("System.ServiceModel", "4.0.0.0")]
    public partial class FTSearchServiceClient : System.ServiceModel.ClientBase<Booben.FTSearchService7.FTSearchService>, Booben.FTSearchService7.FTSearchService {
        
        public FTSearchServiceClient() {
        }
        
        public FTSearchServiceClient(string endpointConfigurationName) : 
                base(endpointConfigurationName) {
        }
        
        public FTSearchServiceClient(string endpointConfigurationName, string remoteAddress) : 
                base(endpointConfigurationName, remoteAddress) {
        }
        
        public FTSearchServiceClient(string endpointConfigurationName, System.ServiceModel.EndpointAddress remoteAddress) : 
                base(endpointConfigurationName, remoteAddress) {
        }
        
        public FTSearchServiceClient(System.ServiceModel.Channels.Binding binding, System.ServiceModel.EndpointAddress remoteAddress) : 
                base(binding, remoteAddress) {
        }
        
        public string StemContent(string contentText) {
            return base.Channel.StemContent(contentText);
        }
        
        public void IndexText(string aliasName, string contentText) {
            base.Channel.IndexText(aliasName, contentText);
        }
        
        public Booben.FTSearchService7.FTSearchRelevantResult SearchPhrase(string phrase, uint minPage, uint maxPage) {
            return base.Channel.SearchPhrase(phrase, minPage, maxPage);
        }
        
        public Booben.FTSearchService7.FTSearchRelevantResult SearchPhraseRel(string phrase, uint minPage, uint maxPage) {
            return base.Channel.SearchPhraseRel(phrase, minPage, maxPage);
        }
        
        public string[] SearchQuery(Booben.FTSearchService7.FTSearchSelector[] selectors, uint minPage, uint maxPage, uint skip, bool agregateBySubject) {
            return base.Channel.SearchQuery(selectors, minPage, maxPage, skip, agregateBySubject);
        }
        
        public Booben.FTSearchService7.FTSearchRelevantResult SearchNewMems(System.DateTime startDate1, System.DateTime startDate2, System.DateTime endDate2) {
            return base.Channel.SearchNewMems(startDate1, startDate2, endDate2);
        }
        
        public string CalculateTrend(string phrase, uint count, uint minPage, uint maxPage) {
            return base.Channel.CalculateTrend(phrase, count, minPage, maxPage);
        }
        
        public Booben.FTSearchService7.FTSearchFTSInstanceInfo GetInfo() {
            return base.Channel.GetInfo();
        }
        
        public Booben.FTSearchService7.FTSearchRelevantResult SearchHtmlSeldomWords(string text) {
            return base.Channel.SearchHtmlSeldomWords(text);
        }
        
        public void SaveIndex() {
            base.Channel.SaveIndex();
        }
    }
}
