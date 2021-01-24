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
public class Load {

    private Load() {
    }

    public static Load Create(Object childObject,
            LoadEnum loadType, // = LoadEnum.LoadRecursively,
            int index) //=0
    {
        Load load = new Load();

        load.ChildObject = childObject;
        load.LoadType = loadType;
        load.Index = index;

        return load;
    }

    public Object ChildObject;
    public LoadEnum LoadType;
    public int Index;
}
