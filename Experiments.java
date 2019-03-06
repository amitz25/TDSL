import java.util.concurrent.ThreadLocalRandom;
import java.util.ArrayList;
import java.util.concurrent.atomic.AtomicInteger;

public class Experiments {
	public static final int WARM_UP_NUM_KEYS = 100000;
	public static final int MIN_KEY_VAL = 1;
	public static final int MAX_KEY_VAL = 1000000;
	public static final int TIMEOUT = 10;
	
	public enum WorkloadType {
	    READ_ONLY(0), MIXED(1), UPDATE_ONLY(2);
	    private final int value;

	    private WorkloadType(int value) {
	        this.value = value;
	    }

	    public int getValue() {
	        return value;
	    }
	}
	
	public enum OperationType {
		REMOVE,
		INSERT,
		CONTAINS
	}
	
	public static int getRand(int min, int max) {
		return ThreadLocalRandom.current().nextInt(min, max + 1);
	}
	
	public static void warmUp(LinkedList ll) {
		System.out.println("Warming up...");
		String object = "dummy";
		for (int i = 0; i < WARM_UP_NUM_KEYS; i++) {
			TX.TXbegin();
			ll.put(getRand(MIN_KEY_VAL, MAX_KEY_VAL), object);
			TX.TXend();
		}
		System.out.println("Finished");
	}
	
	public static void chooseOps(int workloadType, int numOps, ArrayList<OperationType> outOps) {
		if (workloadType == WorkloadType.READ_ONLY.value) {
			for (int i = 0; i < numOps; ++i) {
				outOps.add(OperationType.CONTAINS);
			}
		} else if (workloadType == WorkloadType.UPDATE_ONLY.value) {
			int halfPoint = (int)(numOps / 2.0);
	        for (int i = 0; i < halfPoint; i++) {
	            outOps.add(OperationType.INSERT);
	        }
	        for (int i = halfPoint; i < numOps; i++) {
	            outOps.add(OperationType.REMOVE);
	        }
		} else if (workloadType == WorkloadType.MIXED.value) {
			int halfPoint = (int)(numOps / 2.0);
	        int threeQuarters = (int)(numOps * 3.0 / 4.0);
	        for (int i = 0; i < halfPoint; i++) {
	            outOps.add(OperationType.CONTAINS);
	        }
	        for (int i = halfPoint; i < threeQuarters; i++) {
	            outOps.add(OperationType.REMOVE);
	        }
	        for (int i = threeQuarters; i < numOps; i++) {
	            outOps.add(OperationType.INSERT);
	        }
		}
	}
	
	public static class Worker implements Runnable {
		LinkedList ll;
		int workloadType;
		AtomicInteger opsCounter;
		AtomicInteger abortCounter;
		long endTime;
		
		Worker(LinkedList linkedList, int wType, AtomicInteger opsCount, 
			   AtomicInteger abortCount, long eTime) {
			ll = linkedList;
			workloadType = wType;
			opsCounter = opsCount;
			abortCounter = abortCount;
			endTime = eTime;
		}
		
		public void performOp(OperationType opType, int key) {
			if (opType == OperationType.CONTAINS) {
				ll.containsKey(key);
			} else if (opType == OperationType.INSERT) {
				String dummy = "dummy";
				ll.put(key, dummy);
			} else if (opType == OperationType.REMOVE) {
				ll.remove(key);
			}
		}
		
		@Override
		public void run() {
			while (System.currentTimeMillis() < endTime) {
				int numOps = getRand(1, 7);
				
				ArrayList<OperationType> opTypes = new ArrayList<OperationType>();
				chooseOps(workloadType, numOps, opTypes);
				
				TX.TXbegin();
				try {
					for (int i = 0; i < numOps; i++) {
						int key = getRand(MIN_KEY_VAL, MAX_KEY_VAL);
						performOp(opTypes.get(i), key);
					}
					TX.TXend();
					opsCounter.incrementAndGet();
				} catch (TXLibExceptions.AbortException exp) {
                    abortCounter.incrementAndGet();
                }
			}
		}
	}
	
	public static void main(String[] args) {
		if (args.length != 2) {
			System.out.println("Invalid number of parameters");
			System.out.println("Usage: Experiments <WORKLOAD_TYPE> <NUM_THREADS>");
			System.out.println("Workload types: 0 = READ_ONLY, 1 = MIXED, 2 = UPDATE_ONLY");
			return;
		}
		
		int workloadType = Integer.parseInt(args[0]);
		int numThreads = Integer.parseInt(args[1]);
		AtomicInteger opsCounter = new AtomicInteger(0);
		AtomicInteger abortCounter = new AtomicInteger(0);
		long endTime = System.currentTimeMillis() + TIMEOUT * 1000;
		
		LinkedList ll = new LinkedList();
		warmUp(ll);
		
		System.out.println("Running workers...");
		ArrayList<Thread> threads = new ArrayList<Thread>();
		
		for (int i = 0; i < numThreads; i++) {
			Thread t = new Thread(new Worker(ll, workloadType, opsCounter, abortCounter, endTime)); 
			t.start();
			threads.add(t);
		} 
		
		for (int i = 0; i < numThreads; i++) {
			try {
				threads.get(i).join();
			} catch (InterruptedException exp) {
            System.out.println("InterruptedException thrown!");
            }
		}
		
		System.out.println("Num ops: " + opsCounter.toString());
		System.out.println("Num aborts: " + abortCounter.toString());
	}
}