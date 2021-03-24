import java.util.Scanner;

/**
 * CaptainOliver
 */
public class CaptainOliver {
  private static String welcome =  "Hello, this is your captain speaking. You can call me Captain Oliver.\n" +
                            "Our ship has crashed, and somehow we have ended up in this Labyrinth.\n" +
                            "We must work together to find our way out....";
  
  public static void main(String[] args) {
    Scanner input = new Scanner(System.in);
    System.out.println(welcome);
    input.close();
  }
}