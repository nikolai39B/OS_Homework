#------#
# Data #
#------#
pageReferences = [ 1, 3, 4, 1, 2, 1, 5, 3, 2, 4, 1, 3, 7 ]

def leastRecentlyUsed(pageRefs, numFrames):
	numberOfFaults = 0
	queue = []
	
	# Loop through all references
	for reference in pageRefs:
		# If the reference was in the queue, move it up to the top
		if reference in queue:
			queue.remove(reference)
			queue.insert(0, reference)
			
		# If the reference was not in the queue, add it to the queue
		else:
			queue.insert(0, reference)
			numberOfFaults += 1
			
			# Remove the last element of the queue if necessary
			if len(queue) > numFrames:
				del queue[-1]
				
	return numberOfFaults
	
	
def firstInFirstOut(pageRefs, numFrames):
	numberOfFaults = 0
	queue = []
	
	# Loop through all references
	for reference in pageRefs:
	
		#print("Looking for ", reference, " in ", queue)
	
		# If the reference was in the queue, mark a page fault
		if reference not in queue:
			numberOfFaults += 1
		
			# Add the reference and remove the bottom reference if necessary
			queue.insert(0, reference)
			if len(queue) > numFrames:
					del queue[-1]
				
				
	return numberOfFaults
	
	
def optimal(pageRefs, numFrames):
	numberOfFaults = 0
	queue = []
	
	# Loop through all references
	index = 0
	for reference in pageRefs:
	
		# If the reference was in the queue, mark a page fault
		if reference not in queue:
			numberOfFaults += 1
			
			if len(queue) >= numFrames:
				# Determine which page will not be used for the longest time
				queueItemsPlacesToNextReference = []
				for queueReference in queue:
					queueItemsPlacesToNextReference.append(
						getPlacesToNextReference(pageRefs, queueReference, index))
						
				maxIndex = -1
				maxValue = -1
				for ii in range(len(queueItemsPlacesToNextReference)):
					# If the item here will never be referenced again, use it
					if queueItemsPlacesToNextReference[ii] == -1:
						maxIndex = ii
						break;
						
					# If the item here is higher than the current max, select it
					elif queueItemsPlacesToNextReference[ii] > maxValue:
						maxIndex = ii
						maxValue = queueItemsPlacesToNextReference[ii]
						
				#print (queue)
				#print (queueItemsPlacesToNextReference)
						
				del queue[maxIndex]
			
			queue.insert(0, reference)
					
		index += 1
		
	return numberOfFaults
	
	
def getPlacesToNextReference(pageRefs, reference, startingIndex):
	placesToNextReference = 0
	
	# Loop through the remaining reference
	for ii in range(startingIndex, len(pageRefs)):
	
		# If we've found the reference, return
		if pageRefs[ii] == reference:
			return placesToNextReference
		
		placesToNextReference += 1
		
	# Will not be referenced again
	return -1
	
	
def main():
	for ii in range(4, 5):
		print("Faults with page size = ", ii)
		print("LRU:     ", leastRecentlyUsed(pageReferences, ii))
		print("FIFO:    ", firstInFirstOut(pageReferences, ii))
		print("Optimal: ", optimal(pageReferences, ii))
		
main()
				