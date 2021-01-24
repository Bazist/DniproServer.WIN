/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */
package DniproClient;

public class Change {

    private Change() {
    }

    public static Change Create(Object childObject,
            ChangeEnum changeType, // = ChangeEnum.UpdateRecursively,
            int index) // = 0
    {
        Change change = new Change();

        change.ChildObject = childObject;
        change.ChangeType = changeType;
        change.Index = index;

        return change;
    }

    public Object ChildObject;
    public ChangeEnum ChangeType;
    public int Index;
};
