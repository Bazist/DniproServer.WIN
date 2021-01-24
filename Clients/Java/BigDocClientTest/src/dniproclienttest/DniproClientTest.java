/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */
package dniproclienttest;
import DniproClient.DniproDB;

/**
 *
 * @author Slavik
 */
public class DniproClientTest {

    /**
     * @param args the command line arguments
     */
    public static void main(String[] args) {
        // TODO code application logic here
        
        DniproDB db = new DniproDB("localhost", 4477);
        
        String result = db.Exec("db.GetAll().Count()");
        
        System.out.println(result);
    }
    
}
