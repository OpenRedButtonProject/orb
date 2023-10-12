package org.orbtv.voicerecognitionservice;

import java.time.LocalDateTime;
import java.time.ZoneOffset;
import java.time.ZonedDateTime;
import java.time.format.DateTimeFormatter;

import android.text.TextUtils;
import android.util.ArraySet;

import java.util.ArrayList;
import java.util.Arrays;
import java.util.Collections;
import java.util.HashMap;
import java.util.List;
import java.util.Map;
import java.util.Objects;
import java.util.Stack;

public class CmdParser {
    public static final int INTENT_MEDIA_PAUSE = 0;
    public static final int INTENT_MEDIA_PLAY = 1;
    public static final int INTENT_MEDIA_FAST_FORWARD = 2;
    public static final int INTENT_MEDIA_FAST_REVERSE = 3;
    public static final int INTENT_MEDIA_STOP = 4;
    public static final int INTENT_MEDIA_SEEK_CONTENT = 5;
    public static final int INTENT_MEDIA_SEEK_RELATIVE = 6;
    public static final int INTENT_MEDIA_SEEK_LIVE = 7;
    public static final int INTENT_MEDIA_SEEK_WALLCLOCK = 8;
    public static final int INTENT_SEARCH = 9;
    public static final int INTENT_DISPLAY = 10;
    public static final int INTENT_PLAYBACK = 11;
    public static final int ACT_REQUEST_MEDIA_DESCRIPTION = 18;
    public static final int ACT_REQUEST_TEXT_INPUT = 19;
    public static final int ACT_PRESS_BUTTON_NUMB_ZERO = 20;
    public static final int ACT_PRESS_BUTTON_RED = 30;
    public static final int ACT_PRESS_BUTTON_GREEN = 31;
    public static final int ACT_PRESS_BUTTON_YELLOW = 32;
    public static final int ACT_PRESS_BUTTON_BLUE = 33;
    public static final int ACT_PRESS_BUTTON_UP = 34;
    public static final int ACT_PRESS_BUTTON_DOWN = 35;
    public static final int ACT_PRESS_BUTTON_LEFT = 36;
    public static final int ACT_PRESS_BUTTON_RIGHT = 37;
    public static final int ACT_PRESS_BUTTON_ENTER = 38;
    public static final int ACT_PRESS_BUTTON_BACK = 39;
    public static final int LOG_MESSAGE = 99;
    public static final int LOG_ERROR_NONE_ACTION = 100;
    public static final int LOG_ERROR_MULTI_ACTIONS = 101;
    public static final int LOG_ERROR_INTENT_SEND = 102;
    private final String KW_ANCHOR_NONE = "none";
    private final String KW_ANCHOR_START = "start";
    private final String KW_ANCHOR_END = "end";
    private final String KW_ANCHOR_LIVE = "live";
    private final int KW_DIRECTION_FORWARDS = 1;
    private final int KW_DIRECTION_BACKWARDS = -1;
    private final int KW_TIME_FORMAT_OTHERS = 0;
    private final int KW_TIME_FORMAT_NUMBER = 1;
    private final int KW_TIME_FORMAT_OF = 2;
    private final int KW_TIME_FORMAT_FRAGMENT = 3;
    private final int KW_UNDEFINED = -1;
    private final ArrayList<String> DIRECTION_FORWARDS = new ArrayList<>(
            Arrays.asList("forwards", "after", "later"));
    private final ArrayList<String> DIRECTION_BACKWARDS = new ArrayList<>(
            Arrays.asList("backwards", "back", "before", "ago"));
    private final ArrayList<String> CLOCK_TIMING = new ArrayList<>(
            Arrays.asList("time", "am", "pm", "o'clock", "midday", "noon", "midnight"));
    private final HashMap<String, Integer> SINGLE_ACTIONS = new HashMap<String, Integer>() {
        {
            put("pause", INTENT_MEDIA_PAUSE);
            put("fastforward", INTENT_MEDIA_FAST_FORWARD);
            put("fastreverse", INTENT_MEDIA_FAST_REVERSE);
            put("rewind", INTENT_MEDIA_FAST_REVERSE);
            put("stop", INTENT_MEDIA_STOP);
            put("search", INTENT_SEARCH);
            put("display", INTENT_DISPLAY);
            put("red", ACT_PRESS_BUTTON_RED);
            put("green", ACT_PRESS_BUTTON_GREEN);
            put("yellow", ACT_PRESS_BUTTON_YELLOW);
            put("blue", ACT_PRESS_BUTTON_BLUE);
            put("up", ACT_PRESS_BUTTON_UP);
            put("down", ACT_PRESS_BUTTON_DOWN);
            put("left", ACT_PRESS_BUTTON_LEFT);
            put("right", ACT_PRESS_BUTTON_RIGHT);
            put("ok", ACT_PRESS_BUTTON_ENTER);
            put("okay", ACT_PRESS_BUTTON_ENTER);
            put("back", ACT_PRESS_BUTTON_BACK);
        }
    };

    private final HashMap<String, Integer[]> MULTI_ACTIONS = new HashMap<String, Integer[]>() {
        {
            put("play", new Integer[]{INTENT_MEDIA_PLAY, INTENT_PLAYBACK});
            put("go", new Integer[]{INTENT_MEDIA_SEEK_CONTENT, INTENT_MEDIA_SEEK_RELATIVE, INTENT_MEDIA_SEEK_LIVE, INTENT_MEDIA_SEEK_WALLCLOCK});
            put("skip", new Integer[]{INTENT_MEDIA_SEEK_CONTENT, INTENT_MEDIA_SEEK_RELATIVE, INTENT_MEDIA_SEEK_LIVE, INTENT_MEDIA_SEEK_WALLCLOCK});
            put("jump", new Integer[]{INTENT_MEDIA_SEEK_CONTENT, INTENT_MEDIA_SEEK_RELATIVE, INTENT_MEDIA_SEEK_LIVE, INTENT_MEDIA_SEEK_WALLCLOCK});
            put("enter", new Integer[]{ACT_REQUEST_TEXT_INPUT, ACT_PRESS_BUTTON_ENTER});
        }
    };

    private final Map<String, Integer> NUMBER_NAMES = new HashMap<String, Integer>() {
        {
            put("zero", 0);
            put("one", 1);
            put("two", 2);
            put("three", 3);
            put("four", 4);
            put("five", 5);
            put("six", 6);
            put("seven", 7);
            put("eight", 8);
            put("nine", 9);
            put("ten", 10);
            put("eleven", 11);
            put("twelve", 12);
            put("thirteen", 13);
            put("fourteen", 14);
            put("fifteen", 15);
            put("sixteen", 16);
            put("seventeen", 17);
            put("eighteen", 18);
            put("nineteen", 19);
            put("twenty", 20);
            put("thirty", 30);
            put("forty", 40);
            put("fifty", 50);
            put("sixty", 60);
            put("seventy", 70);
            put("eighty", 80);
            put("ninety", 90);
            put("hundred", 100);
        }
    };

    public Command parseIncomingCommand(String cmd) {
        // parse cmd
        List<String> words = decomposeCommand(cmd);
        if (words == null || words.isEmpty()) {
            return handleResult(LOG_ERROR_NONE_ACTION);
        }
        ArraySet<Integer> actions = new ArraySet<>();
        List<Command> validActions = new ArrayList<>();
        List<Integer> basicActions = new ArrayList<>();
        for (int i = 0; i < words.size(); ++i) {
            String w = words.get(i);
            // handling cases with verb + ing
            if (w.equals("playing")) {
                words.add("play");
            } else if (w.matches("^.*ing$")) {
                String substring = w.substring(0, w.length() - "ing".length());
                words.add(substring);
                words.add(substring + "e");
                words.add(substring.substring(0, substring.length() - 1));
                continue;
            }
            if (SINGLE_ACTIONS.containsKey(w)) {
                actions.add(SINGLE_ACTIONS.get(w));
            }
            if (MULTI_ACTIONS.containsKey(w)) {
                actions.addAll(Arrays.asList(Objects.requireNonNull(MULTI_ACTIONS.get(w))));
            }
        }
        if (actions.isEmpty()) {
            // check action for number button
            basicActions.addAll(findNumberButtonActions(words));
        }

        // check valid actions
        for (Integer a : actions) {
            Command newCommand = new Command(a, words);
            if (isValidAction(newCommand)) {
                validActions.add(newCommand);
            } else if (isValidBasicAction(a)) {
                basicActions.add(a);
            }
        }
        if (validActions.isEmpty() && basicActions.isEmpty()) {
            if (words.contains("what") && words.contains("watch")) {
                return new Command(ACT_REQUEST_MEDIA_DESCRIPTION);
            }
            return handleResult(LOG_ERROR_NONE_ACTION);
        } else if (validActions.size() > 1) {
            return handleResult(LOG_ERROR_MULTI_ACTIONS);
        } else if (validActions.size() == 1) {
            return handleOneValidIntent(validActions.get(0));
        }
        return handleBasicActions(basicActions);
    }

    private Command handleBasicActions(List<Integer> actions) {
        if (actions.isEmpty()) {
            return handleResult(LOG_ERROR_NONE_ACTION);
        } else if (actions.size() == 1) {
            return new Command(actions.get(0));
        } else if (actions.size() > 2) {
            return handleResult(LOG_ERROR_MULTI_ACTIONS);
        }
        boolean isStopAction = false;
        List<Integer> vcrActions = new ArrayList<>();
        for (int i = 0; i < actions.size(); ++i) {
            Integer act = actions.get(i);
            switch (act) {
                case INTENT_MEDIA_STOP:
                    isStopAction = true;
                    break;
                case INTENT_MEDIA_PAUSE:
                case INTENT_MEDIA_PLAY:
                case INTENT_MEDIA_FAST_FORWARD:
                case INTENT_MEDIA_FAST_REVERSE:
                    vcrActions.add(act);
                    break;
            }
        }
        if (isStopAction && vcrActions.size() == 1) {
            if (vcrActions.get(0) == INTENT_MEDIA_PLAY) {
                // e.g. "stop playing"
                return new Command(INTENT_MEDIA_STOP);
            }
            // e.g. "stop fast-forwarding"
            return new Command(INTENT_MEDIA_PLAY);
        }
        return handleResult(LOG_ERROR_MULTI_ACTIONS);
    }

    private List<String> decomposeCommand(String cmd) {
        if (cmd == null) {
            return null;
        }
        return new ArrayList<>(Arrays.asList(
                cmd.toLowerCase().replaceAll("[^\\sa-zA-Z0-9]", " ")
                        .replaceAll("\\s{2,}", " ")
                        .replaceAll("(^|\\s)fast\\s(forward|forwarding)($|\\s)", " fastforward ")
                        .replaceAll("(^|\\s)fast\\s(reverse|reversing)($|\\s)", " fastreverse ")
                        .replaceAll("(^|\\s)o\\sclock($|\\s)", " o'clock ")
                        .replaceAll("(^|\\s)a\\sm($|\\s)", " am ")
                        .replaceAll("(^|\\s)p\\sm($|\\s)", " pm ")
                        .replaceAll("\\s{2,}", " ")
                        .trim().split("\\s+")));
    }


    private Command handleOneValidIntent(Command cmd) {
        if (cmd == null) {
            return null;
        }
        cmd.offset = cmd.direction * cmd.offset;
        cmd.direction = KW_DIRECTION_FORWARDS;
        switch (cmd.actId) {
            case INTENT_MEDIA_SEEK_LIVE:
                if (cmd.offset > 0) {
                    break;
                }
                return cmd;
            case INTENT_MEDIA_SEEK_WALLCLOCK:
                // TODO - check wall clock format is valid
                cmd.item = cmd.clock;
                return cmd;
            case INTENT_SEARCH:
            case INTENT_DISPLAY:
            case ACT_REQUEST_TEXT_INPUT:
                if (cmd.item == null) {
                    break;
                }
                return cmd;
            case INTENT_PLAYBACK:
                if (cmd.item == null) {
                    break;
                }
                if ((cmd.anchor.equals(KW_ANCHOR_START) || cmd.anchor.equals(KW_ANCHOR_END)) &&
                        cmd.offset != 0) {
                    return cmd;
                } else if (cmd.anchor.equals(KW_ANCHOR_LIVE) && cmd.offset <= 0) {
                    cmd.anchor = "";
                    return cmd;
                } else {
                    cmd.anchor = "";
                    cmd.offset = -999999;
                    return cmd;
                }
        }
        return cmd;
    }

    private boolean isValidBasicAction(Integer actionId) {
        if (actionId <= INTENT_MEDIA_STOP) {
            return true;
        }
        return actionId >= ACT_PRESS_BUTTON_RED && actionId <= ACT_PRESS_BUTTON_ENTER;
    }

    private Command handleResult(int code) {
        Command resultCmd = new Command(code);
        switch (code) {
            case LOG_ERROR_NONE_ACTION:
                resultCmd.item = "Error: no valid action in command is found";
                break;
            case LOG_ERROR_MULTI_ACTIONS:
                resultCmd.item = "Error: more than one action is found in one command";
                break;
            case LOG_ERROR_INTENT_SEND:
                resultCmd.item = "Error: an invalid Intent cannot be sent";
                break;
            default:
                resultCmd.item = "Error: others";
        }
        return resultCmd;
    }

    private boolean isValidAction(Command cmd) {
        switch (cmd.actId) {
            case INTENT_MEDIA_SEEK_CONTENT:
                return (cmd.anchor.equals(KW_ANCHOR_START) || cmd.anchor.equals(KW_ANCHOR_END));
            case INTENT_MEDIA_SEEK_RELATIVE:
                // TODO - check offset could be 0
                return cmd.anchor.equals(KW_ANCHOR_NONE) && cmd.offset != 0;
            case INTENT_MEDIA_SEEK_LIVE:
                return cmd.anchor.equals(KW_ANCHOR_LIVE);
            case INTENT_MEDIA_SEEK_WALLCLOCK:
                return cmd.clock != null;
            case INTENT_SEARCH:
            case INTENT_DISPLAY:
            case INTENT_PLAYBACK:
            case ACT_REQUEST_TEXT_INPUT:
                return cmd.item != null;
            case ACT_PRESS_BUTTON_BACK:
                return cmd.anchor.equals(KW_ANCHOR_NONE) && cmd.offset == 0;
            default:
                return false;
        }
    }

    private List<Integer> findNumberButtonActions(List<String> words) {
        ArrayList<Integer> numbers = new ArrayList<>();
        for (String w : words) {
            if (NUMBER_NAMES.containsKey(w) || w.matches("^\\d+$")) {
                int num = parseNumberName(Collections.singletonList(w));
                if (0 <= num && num < 10) {
                    numbers.add(num + ACT_PRESS_BUTTON_NUMB_ZERO);
                }
            }
        }
        return numbers;
    }

    private double parseNumberFormat(List<String> words) {
        double number = parseSpecialRules(words);
        if (number != KW_UNDEFINED) {
            return number;
        }
        // parse time formats, e.g. "30 minutes" and "a half of an hour"
        ArrayList<Integer> formatList = new ArrayList<>();
        // find the position of fragment word "half" or "quarter"
        int fragmentPos = KW_UNDEFINED;
        for (int i = words.size() - 1; i >= 0; --i) {
            String curr = words.get(i);
            int currFormat = checkFormat(curr);
            fragmentPos = (currFormat == KW_TIME_FORMAT_FRAGMENT &&
                    fragmentPos == KW_UNDEFINED) ? i : fragmentPos;
            if (formatList.isEmpty() || (formatList.get(formatList.size() - 1) != currFormat)) {
                formatList.add(currFormat);
            }
        }
        String format = TextUtils.join("", formatList);
        if (fragmentPos == KW_UNDEFINED) {
            // for case, num hour
            return parseNumberName(words);
        }
        double factor = convertFragment(words.get(fragmentPos));
        if (format.matches("^1231.*")) {
            // for case, first_num half of second_num hour
            double first = parseNumberName(words.subList(0, fragmentPos));
            double second = parseNumberName(words);
            return (first * factor * second);
        } else if (format.matches("^31.*")) {
            // for case, first_num half hour
            double first = parseNumberName(words.subList(0, fragmentPos));
            return (first * factor);
        } else if (format.matches("^3.*")) {
            // for case, half hour
            return (factor);
        } else if (format.matches("^1.*")) {
            // for case, num hour
            return parseNumberName(words);
        }
        return 0;
    }

    private double parseSpecialRules(List<String> words) {
        String str = String.join(" ", words);
        if (str.matches(".*(half)\\s((a)n?|one|1)$")) {
            // for case, half (a) hour
            return 0.5;
        } else if (str.matches(".*(and)\\s(((a)n?|one|1)\\s)?(hal(f|ves))$") ||
                str.matches(".*(and)\\s(((a)n?|one|1)\\s)?(quarter(s?))$") ||
                str.matches(".*(and)\\s(three)\\s(quarter(s?))$")) {
            // find the position of keyword "and"
            int andPos = words.indexOf("and");
            double fragment = convertFragment(str);
            return fragment + parseNumberName(words.subList(0, andPos));
        }
        return KW_UNDEFINED;
    }

    private int parseNumberName(List<String> words) {
        // e.g. convert {"twenty", "five"} to 25
        int sum = 0;
        if (words == null || words.size() < 1) {
            return sum;
        }
        int prev = 0;
        int curr;
        for (int i = 0; i < words.size(); ++i) {
            curr = 0;
            String currWord = words.get(i);
            if (currWord.matches("^\\d+$")) {
                curr = Integer.parseInt(currWord);
            } else if (currWord.matches("^(a)n?$")) {
                curr = 1;
            } else if (NUMBER_NAMES.containsKey(currWord)) {
                curr = NUMBER_NAMES.get(currWord);
            } else if (!currWord.equals("and")) {
                sum = 0;
                prev = 0;
            }
            if (curr < 20) {
                sum += curr;
                prev += curr;
            } else if (curr < 100) {
                sum += curr;
                prev = curr;
            } else {
                sum -= prev;
                sum += prev * curr;
                prev = 0;
            }
        }
        return sum;
    }

    private String findClockTime(List<String> words) {
        int set = KW_UNDEFINED;
        Stack<String> numbers = new Stack<>();
        Stack<String> preNumbers = new Stack<>();
        for (int i = 0; i < words.size(); ++i) {
            String currWord = words.get(i);
            if (currWord.matches("^-?\\d+$")) {
                numbers.push(currWord);
            } else if (NUMBER_NAMES.containsKey(currWord)) {
                numbers.push(currWord);
            } else if (currWord.equals("pm")) {
                set = 12;
                return parseTimeFormat(numbers, set);
            } else if (currWord.equals("am")) {
                set = 0;
                return parseTimeFormat(numbers, set);
            } else if (currWord.equals("o'clock")) {
                int hr = parseNumberName(words.subList(0, i));
                return convertTimeFormat(hr, 0);
            } else if (currWord.equals("midday") || currWord.equals("noon")) {
                return convertTimeFormat(12, 0);
            } else if (currWord.equals("midnight")) {
                return convertTimeFormat(0, 0);
            } else {
                preNumbers = numbers;
                numbers.clear();
            }
        }
        if (numbers.empty()) {
            return parseTimeFormat(preNumbers, set);
        } else {
            return parseTimeFormat(numbers, set);
        }
    }

    private boolean isSameForms(Stack<String> numbers) {
        if (numbers.empty()) {
            return false;
        } else if (numbers.size() == 1) {
            return true;
        }
        boolean isFirstNumber = numbers.get(0).matches("^-?\\d+$");
        for (int i = 1; i < numbers.size(); ++i) {
            if (numbers.get(i).matches("^-?\\d+$") != isFirstNumber) {
                return false;
            }
        }
        return true;
    }

    private String parseTimeFormat(Stack<String> numbers, int set) {
        if (!isSameForms(numbers)) {
            return null;
        }
        ArrayList<Integer> validNumbers = new ArrayList<>();
        while (!numbers.empty()) {
            String currWord = numbers.pop();
            int size = validNumbers.size();
            if (currWord.matches("^-?\\d+$")) {
                validNumbers.add(Integer.parseInt(currWord));
                continue;
            }
            int curr = 0;
            if (NUMBER_NAMES.containsKey(currWord)) {
                curr = NUMBER_NAMES.get(currWord);
            }
            if (size == 0) {
                validNumbers.add(curr);
                continue;
            }
            if (curr < 20) {
                validNumbers.add(curr);
            } else if (curr < 99) {
                int prev = validNumbers.get(size - 1);
                if (prev < 10) {
                    validNumbers.set(size - 1, prev + curr);
                } else {
                    validNumbers.add(curr);
                }
            }
        }
        int hr;
        int min = 0;
        if (validNumbers.size() == 2) {
            min = validNumbers.get(0);
            hr = validNumbers.get(1);
        } else if (validNumbers.size() == 1) {
            hr = validNumbers.get(0);
        } else {
            return null;
        }
        if (set == KW_UNDEFINED) {
            return convertTimeFormat(hr, min);
        } else if (set == 0 && hr > 12) {
            return null;
        } else if (hr == 12) {
            return convertTimeFormat(hr + set - 12, min);
        } else {
            return convertTimeFormat(hr + set, min);
        }
    }

    private int checkFormat(String curr) {
        if (curr != null) {
            if (curr.matches("^(a)n?$") || curr.equals("and") ||
                    curr.matches("^\\d+$") ||
                    NUMBER_NAMES.containsKey(curr)) {
                return KW_TIME_FORMAT_NUMBER;
            } else if (curr.equals("of")) {
                return KW_TIME_FORMAT_OF;
            } else if (curr.matches("^(quarter)s?$") ||
                    curr.matches("^hal(f|ve)s?$")) {
                return KW_TIME_FORMAT_FRAGMENT;
            }
        }
        return KW_TIME_FORMAT_OTHERS;
    }

    private double convertFragment(String fragment) {
        if (fragment != null) {
            if (fragment.matches(".*(three)\\s(quarter(s?))$")) {
                return 0.75;
            } else if (fragment.matches(".*(quarter)s?$")) {
                return 0.25;
            } else if (fragment.matches(".*hal(f|ve)s?$")) {
                return 0.5;
            }
        }
        return 0;
    }

    private int convertTimeUnit(String time) {
        if (time.matches("^(hour|hr)s?$")) {
            return 60 * 60;
        } else if (time.matches("^(minute|min)s?$")) {
            return 60;
        } else if (time.matches("^(second|sec)s?$")) {
            return 1;
        } else {
            return 0;
        }
    }

    private String convertTimeFormat(int hr, int min) {
        if (hr < 0 || hr > 23 || min < 0 || min > 59) {
            return null;
        }
        LocalDateTime now = LocalDateTime.now();
        int hrOffset = hr - now.getHour();
        return ZonedDateTime.now(ZoneOffset.UTC).plusHours(hrOffset).withMinute(min)
                .withSecond(0).withNano(0).format(DateTimeFormatter.ISO_INSTANT);
    }

    public class Command {
        Integer actId;
        int direction = KW_DIRECTION_FORWARDS;
        String anchor = KW_ANCHOR_NONE;
        int offset = 0;
        String clock = null;
        String item;

        Command(Integer id) {
            actId = id;
        }

        Command(Integer actionId, List<String> words) {
            actId = actionId;
            // For particular intents, get item name from index_start to index_end
            int start = findIndexOfActions(actionId, words);
            int end = words.size();
            for (int i = 0; i < words.size(); ++i) {
                int key = end;
                String w = words.get(i);
                double number;
                if (i >= 1 && w.matches("^(hour|hr|minute|min|second|sec)s?$")) {
                    String pre = words.get(i - 1);
                    if (pre == null) {
                        continue;
                    }
                    if (pre.matches("^-?\\d+$")) {
                        number = Integer.parseInt(pre);
                    } else {
                        number = parseNumberFormat(words.subList(0, i));
                    }
                    double n = number * convertTimeUnit(w);
                    offset += Math.round(n);
                    int indexOfTime = findFirstIndexOfTimeFormat(words.subList(0, i));
                    key = Math.min(indexOfTime, i);
                } else if (DIRECTION_FORWARDS.contains(w)) {
                    direction = KW_DIRECTION_FORWARDS;
                    key = Math.min(i, key);
                } else if (DIRECTION_BACKWARDS.contains(w)) {
                    direction = KW_DIRECTION_BACKWARDS;
                    key = Math.min(i, key);
                } else if (CLOCK_TIMING.contains(w)) {
                    clock = findClockTime(words);
                }
                if (key > start) {
                    end = key;
                }
                // For seek intents, get keywords for time anchor
                switch (w) {
                    case "start":
                        anchor = KW_ANCHOR_START;
                        break;
                    case "end":
                        anchor = KW_ANCHOR_END;
                        break;
                    case "live":
                    case "ago":
                        anchor = KW_ANCHOR_LIVE;
                        break;
                }
            }
            if (start == KW_UNDEFINED || start >= end) {
                item = null;
            } else if (actionId == INTENT_SEARCH || actionId == INTENT_DISPLAY ||
                    actionId == ACT_REQUEST_TEXT_INPUT) {
                item = TextUtils.join(" ", words.subList(start, words.size()));
            } else {
                // actionId == INTENT_MEDIA_PLAY
                item = TextUtils.join(" ", words.subList(start, end));
            }
        }

        private int findIndexOfActions(Integer actionId, List<String> words) {
            int start;
            switch (actionId) {
                case INTENT_SEARCH:
                    start = words.indexOf("search") + 1;
                    break;
                case INTENT_DISPLAY:
                    start = words.indexOf("display") + 1;
                    break;
                case INTENT_PLAYBACK:
                    start = words.indexOf("play") + 1;
                    break;
                case ACT_REQUEST_TEXT_INPUT:
                    start = words.indexOf("enter") + 1;
                    break;
                default:
                    start = KW_UNDEFINED;
            }
            return start;
        }

        private int findFirstIndexOfTimeFormat(List<String> words) {
            for (int i = words.size() - 1; i >= 0; --i) {
                if (checkFormat(words.get(i)) == KW_TIME_FORMAT_OTHERS) {
                    return i;
                }
            }
            return words.size();
        }
    }
}
