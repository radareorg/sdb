using SDB;

public class Test {
	public static void Main() {
		var a = new Sdb ();
		a.Set ("Hello", "World");
		System.Console.WriteLine ("Hello {0}", a.Get ("Hello"));
	}
}
