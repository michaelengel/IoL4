// by Kevin Glen Roy Greer

/*
Sample Usage:

DaysOfWeekEnum = Enum clone do (
    type      = "DayOfWeekEnum"

    SUNDAY    = value("Sunday")
    MONDAY    = value("Monday")
    TUESDAY   = value("Tuesday")
    WEDNESDAY = value("Wednesday")
    THURSDAY  = value("Thursday")
    FRIDAY    = value("Friday")
    SATURDAY  = value("Saturday")
)

DaysOfWeekEnum SUNDAY
DaysOfWeekEnum print
*/

// A possible value of an Enumeration
EnumValue = Object clone do (

    init  = method(enum, label,
        self parent = enum
        self label  = label
        self index  = count
    )

    print = method(
        index print
        " " print
        label print
    )
)


// An Enumeration of EnumValue's
Enum = Object clone do (
    type = "Enum"

    init = method(
        self parent = List clone
        self extent = parent
    )

    value = method(label,
        e = EnumValue clone(self, label, index)
        add(e)

        return e
    )

    print = method(
        type print
        ": " print
        extent foreach(i, val,
           if ( i != 0, ", " print )
           val print
        )
    )
)

