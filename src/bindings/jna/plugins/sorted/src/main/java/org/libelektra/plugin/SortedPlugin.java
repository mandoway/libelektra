package org.libelektra.plugin;

import org.libelektra.*;

import javax.annotation.Nonnull;
import java.util.Comparator;
import java.util.List;
import java.util.Optional;
import java.util.concurrent.atomic.AtomicBoolean;
import java.util.stream.Collectors;

public class SortedPlugin implements Plugin {

    private static final String PLUGIN_NAME = "Sorted";
    private static final String META_SORTED = "check/sorted";
    private static final String META_SORTED_DIRECTION = "check/sorted/direction";

    private enum Direction {
        ASC, DESC
    }

    @Nonnull
    @Override
    public String getName() {
        return PLUGIN_NAME;
    }

    @Override
    public int open(KeySet config, Key errorKey) {
        throw new UnsupportedOperationException();
    }

    @Override
    public int get(KeySet keySet, Key parentKey) {
        if (isPluginMetadataRequested(parentKey)) {
            SortedMetadata.appendAllTo(keySet);
        }

        // todo add validation and possibly a warning

        return STATUS_SUCCESS;
    }

    private boolean isPluginMetadataRequested(Key parentKey) {
        return parentKey.isBelowOrSame(Key.create(PROCESS_CONTRACT_ROOT));
    }

    @Override
    public int set(KeySet keySet, Key parentKey) throws KDBException {
        return validateSorted(keySet, parentKey) ? STATUS_ERROR : STATUS_SUCCESS;
    }

    private boolean validateSorted(KeySet keySet, Key parentKey) {
        AtomicBoolean foundError = new AtomicBoolean(false);

        keySet.forEach(key -> {
            Optional<ReadableKey> sortedMeta = key.getMeta(META_SORTED);

            if (sortedMeta.isPresent()) {
                String sortedKeyValue = sortedMeta.get().getString(); // TODO: Primitive/Complex types

                Direction direction = Direction.ASC;    // TODO: Use direction
                try {
                    direction = getDirection(key);
                } catch (IllegalArgumentException e) {
                    parentKey.setError(
                            ErrorCode.VALIDATION_SEMANTIC,
                            "Invalid direction for sorted plugin in key '" + key.getName() + "':" + e.getMessage());
                    foundError.set(true);
                }

                List<Key> arrayKeys = getSortedArrayKeys(keySet, key);

                if (!isSorted(arrayKeys)) {
                    parentKey.setError(
                            ErrorCode.VALIDATION_SEMANTIC,
                            "Values are not sorted below key '" + key.getName() + "'");
                    foundError.set(true);
                }
            }
        });
        return foundError.get();
    }

    private boolean isSorted(List<Key> arrayKeys) {
        return arrayKeys.stream()
                .sorted(Comparator.comparing(ReadableKey::getString))
                .collect(Collectors.toList())
                .equals(arrayKeys);
    }

    private List<Key> getSortedArrayKeys(KeySet keySet, Key parentKey) {
        return keySet.stream()
                .filter(it -> it.getName().startsWith(parentKey.getName() + "/#"))
                .sorted((o1, o2) -> {
                    int index1 = parseArrayIndex(o1.getName(), parentKey.getName());
                    int index2 = parseArrayIndex(o2.getName(), parentKey.getName());

                    if (index1 == index2)
                        return 0;

                    return index1 < index2 ? -1 : 1;
                })
                .collect(Collectors.toList());
    }

    private int parseArrayIndex(String key, String prefix) {
        return Integer.parseInt(
                key
                        .substring(prefix.length() + 1)
                        .split("/")[0]
                        .replaceAll("#", "")
                        .replaceAll("_", "")
        );
    }

    private Direction getDirection(Key key) throws IllegalArgumentException {
        Optional<ReadableKey> sortedDirectionMeta = key.getMeta(META_SORTED_DIRECTION);
        Direction direction = Direction.ASC;

        if (sortedDirectionMeta.isPresent()) {
            String sortedDirectionKey = sortedDirectionMeta.get().getString();

            if (Direction.DESC.toString().toLowerCase().equals(sortedDirectionKey)) {
                direction = Direction.DESC;
            } else if (!Direction.ASC.toString().toLowerCase().equals(sortedDirectionKey)) {
                throw new IllegalArgumentException(sortedDirectionKey);
            }
        }

        return direction;
    }

    @Override
    public int error(KeySet keySet, Key parentKey) {
        throw new UnsupportedOperationException();
    }

    @Override
    public int close(Key parentKey) {
        throw new UnsupportedOperationException();
    }
}
