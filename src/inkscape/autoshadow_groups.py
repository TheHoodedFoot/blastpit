def interval_pair_partition(pairs):
    # Sort the pairs based on their end points
    pairs.sort(key=lambda x: x[1])

    # Initialize the result with the first pair
    result = [[pairs[0]]]

    # Iterate over the remaining pairs
    for pair in pairs[1:]:
        # Check if the current pair can be added to any existing set
        added = False
        for interval_set in result:
            if pair[0] >= interval_set[-1][1]:
                interval_set.append(pair)
                added = True
                break

        # If the current pair cannot be added to any existing set, create a new set
        if not added:
            result.append([pair])

    return result


# Example usage:
pairs = [
    (1, 3),
    (2, 4),
    (5, 7),
    (6, 8),
    (9, 10),
    (1.0, 2.5),
    (3.0, 4.5),
    (2.0, 3.5),
    (6.0, 7.5),
]
print(interval_pair_partition(pairs))
