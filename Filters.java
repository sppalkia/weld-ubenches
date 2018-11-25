
public class Filters {

  public static void main(String[] args) {

    int size = (200 * 1000 * 1000);

    if (args.length > 1) {
      size = Integer.parseInt(args[1]);
    }

    System.out.print("Generating data...");
    Dataset dataset = new Dataset(size);
    System.out.println("done.");

    long average = 0;
    long trials = 20;

    for (int i = 0; i < trials; i++) {
      long startTime = System.nanoTime();
      long result = dataset.run();
      long endTime = System.nanoTime();

      long ms = (endTime - startTime) / 1000000;
      average += ms;

      System.out.println("\t Time: " + ms + " Result: " + result);
    }

    System.out.println("Average runtime: " + average / trials);
  }
}

class Column {
  public int values[];
  public byte nulls[];

  public Column(int size, int multiplier) {

    this.values = new int[size];
    this.nulls = new byte[size];

    for (int i = 0; i < size; i++) {
      this.values[i] = (int)(Math.random() * (double)multiplier);
      this.nulls[i] = 0;
    }
  }
}

class Dataset {

  public Column c1, c2, c3, c4;
  long size;

  public Dataset(int size) {
    this.c1 = new Column(size, 1);
    this.c2 = new Column(size, 10);
    this.c3 = new Column(size, 100);
    this.c4 = new Column(size, 1000);

    this.size = size;
  }

  public long run() {
    long count = 0;
    for (int i = 0; i < this.size; i++) {
      if (c1.nulls[i] == 0 && c1.values[i] < 0) {
        if (c2.nulls[i] == 0 && c2.values[i] < 0) {
          if (c3.nulls[i] == 0 && c3.values[i] < 0) {
            if (c4.nulls[i] == 0 && c4.values[i] < 0) {
              count++;
            }
          }
        }
      }
    }
    return count;
  }

}
