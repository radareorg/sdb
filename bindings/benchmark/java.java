import org.json.*;

public class java {
	public static void main(String[] args) {
		System.out.println ("Hello World\n");
		for (int i=0;i<199999;i++) {
			String title = dojson();
			//System.out.println (title);
		}
	}
	public static String dojson() {
		String json = "{\"glossary\":{\"title\":\"example glossary\",\"page\":1,\"GlossDiv\":{\"title\":\"First gloss title\",\"GlossList\":{\"GlossEntry\":{\"ID\":\"SGML\",\"SortAs\":\"SGML\",\"GlossTerm\":\"Standard Generalized Markup Language\",\"Acronym\":\"SGML\",\"Abbrev\":\"ISO 8879:1986\",\"GlossDef\":{\"para\":\"A meta-markup language, used to create markup languages such as DocBook.\",\"GlossSeeAlso\":[\"OK\",\"GML\",\"XML\"]},\"GlossSee\":\"markup\"}}}}}'";
		try {
			JSONObject obj = new JSONObject (json);
			JSONObject g = obj.getJSONObject ("glossary");
			String title = g.getString ("title");
			return title;
		} catch (Exception e) {
			e.printStackTrace ();
		}
		return "";
	}
}
