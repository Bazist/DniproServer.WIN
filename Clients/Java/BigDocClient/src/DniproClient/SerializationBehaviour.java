/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */
package DniproClient;

import java.lang.reflect.InvocationTargetException;
import java.lang.reflect.Method;
import java.lang.reflect.ParameterizedType;
import java.lang.reflect.Type;

/**
 *
 * @author Slavik
 */
public class SerializationBehaviour {

    public Class GetSubType(Object parentObject) throws ClassNotFoundException {
        Class subType;

        Type[] types = ((ParameterizedType) (parentObject.getClass().getGenericSuperclass())).getActualTypeArguments();

        if (types.length > 0) {
            subType = Class.forName(types[0].toString());
        } else {
            subType = String.class;
        }

        return subType;
    }

    public Object CreateSubItem(Object parentObject, Class type) throws InstantiationException, IllegalAccessException {
        return type.newInstance();
    }

    public void AddSubItem(Object parentObject, Object childObject) throws NoSuchMethodException, IllegalAccessException, InvocationTargetException {
        Method mi;

        if (childObject != null) {
            Class[] argTypes = new Class[]{childObject.getClass()};

            mi = parentObject.getClass().getDeclaredMethod("Add", argTypes);
        } else {
            mi = parentObject.getClass().getDeclaredMethod("Add");
        }

        mi.invoke(parentObject,
                new Object[]{childObject});
    }
}
