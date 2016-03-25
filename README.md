# Dnipro Server

**DniproDB** one of the fastest document oriented database in the world. The database has innovated core based on **Trie** structures, **Inverted indexes** and **Lock Free** algorithms. We reinvented standard approaches in core to store **JSONs** only as set of keys.

# Why Dnipro ?
## REALLY EASY SYNTAX

Syntax so important for daily using.
Just put hierarchy of your bigest **Business Object** to database in one row

```C#
db.AddDoc(obj);
```
And that's all ! We indexed the object by all attributes and save it in convinient JSON format.

**Link queries** helps build complex queries

```C#
User[] users = 
db.GetWhere("{'FirstName':'John'})")
  .AndWhere("{'Address':{'Country':'UK'}})"
	  .Select<User>("{'FirstName':$,'LastName':$}");
```

And, of course, perfect **JOIN** ...
```C#
JoinResult[] jrs =
db.GetWhere("{'FirstName':'John'}")
  .Join("{'CarModel':$}","{'Type':'Car', 'Model':$}")
  .Join("{'Company':$}","{'Type':'Company', 'Name':$}")
  .Select<JoinResult>
         ("{'FirstName':$}{'Engine':$}{'City':$}");
```
Few clicks by mouse and database is ready for using in your environment 
(Ready for **Windows** platform and **.NET**, **Java** languages)
Learn more ? Just download our comprehensive [Dnipro In Using](http://booben.com/DniproDB_In_Using_EN.PDF) book.

## SO FAST

Speed, speed and again speed.
Do you remember ? **JSONs** it is just keys in embeded **Key\Value storage**.
So 
* When you insert a document, you just insert keys to Key\Value storage.
* When you update an attribute in document, you just update key in the storage.
* When you insert an attribute in document, you just insert new key in the storage, without any overwriting of document.
* When you select part of document, you just lookup several keys

And ... what you should know ... one operation of insert/update/lookup of long key in storage costs 
about **fifty nanoseconds* on core levelю
In regular benchmarks **Dnipro** faster than standard databases in [ten](http://forum.pikosec.com/viewforum.php?f=7) times and more.

Run in console **db.SelfTest()** to see how many seconds need to your computer to finish 
more than ten millions queries in our InMemory benchmark.

## RELIABLE DATABASE

Without any compromises.

**ACID** and three types of **Transactions** (ReadCommited, RepeatableRead, Snapshot).

Very carefully blocking on **attribute level** gives maximum parallelism.

Many transactions can change one document at the same time. 
If changing attributes are not crossed, they changes without conflicts to each other. 
They wouldn’t overwrite changes to each other.
**Resolve any deadlocks** with full information in log: who, when, what documents, what attributes

And now it's Opensource.

**Enjoy** !
