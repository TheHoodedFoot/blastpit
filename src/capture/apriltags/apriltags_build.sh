#!/bin/sh
FAMILY="TagStandard41h12"
ant && \
java -cp april.jar april.tag.TagFamilyGenerator circle_9 7 && \
java -cp april.jar april.tag.TagToC april.tag.$FAMILY && \
java -cp april.jar april.tag.GenerateTags april.tag.$FAMILY .
