package com.google.errorprone.annotations;

import java.lang.annotation.ElementType;
import java.lang.annotation.Retention;
import java.lang.annotation.RetentionPolicy;
import java.lang.annotation.Target;

/** Present so Cronet's compiled references resolve. Not used by ORB. */
@Retention(RetentionPolicy.CLASS)
@Target({ElementType.TYPE, ElementType.ANNOTATION_TYPE})
public @interface DoNotMock {
    String value() default "Create a real instance instead";
}
