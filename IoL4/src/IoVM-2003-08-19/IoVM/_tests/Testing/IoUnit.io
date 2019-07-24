// 2003 (c) Uladzimir Liashkevich, freeware

// useful addition

Object asString = method ("a " + type)

stdout = (
	nextPutAll = method (aString, write(aString))
	cr = method (write("\n"))
)

List proto do (
	type = "List"

	injectInto = method (initial, twoArgBlock,
		accum = initial
		foreach(i, v, accum = twoArgBlock(accum, v))
		accum)
)

// IoUnit

TestResult = Object clone do (

	type = "TestResult"

	failures = List clone
	errors = List clone
	passed = List clone

	failure = "IOUnit.TestResult.failure"

	init = method (
		self failures = self failures clone
		self errors = self errors clone
		self passed = self passed clone)

	runCase = method (aTestCase,
		//aTestCase runCase
		ok = Nil
		catchException("",
			catchException(TestResult failure,
				aTestCase runCase
				ok = 1,
			// catch (TestResult failure)
				aTestCase setDescription(exceptionDescription)
				failures add(aTestCase)),
		// catch (...)
			aTestCase setDescription(exceptionDescription)
			errors add(aTestCase))
		if (ok, passed add(aTestCase)))

	runCount = method (return failures count + errors count + passed count)

	passedCount = method (passed count)
	failureCount = method (failures count)
	errorCount = method (errors count)

	printMethods = method (aList,
		if (aList count > 0,
			aList foreach(i, each,
				aStream cr
				aStream nextPutAll("    #")
				aStream nextPutAll(each selector)
				aStream nextPutAll(", ")
				aStream nextPutAll(each description))
			aStream cr))

	printOn = method (aStream,
		aStream nextPutAll(self runCount asString)
		aStream nextPutAll(" run, ")

		aStream nextPutAll(self passedCount asString)
		aStream nextPutAll(" passed, ")

		if (passedCount == runCount, Nil, aStream cr) // != doesn't work
		aStream nextPutAll(self failureCount asString)
		aStream nextPutAll(" failed, ")
		printMethods(failures)

		aStream nextPutAll(self errorCount asString)
		aStream nextPutAll(" errors")
		printMethods(errors)

		aStream cr)
)

TestSuite = Object clone do (

	type = "TestSuite"

	name = ""
	tests = List clone

	init = method (
		self tests = tests clone)

	setName = method (aName,
		self name = aName
		self)

	addTest = method (aTest,
		tests add(aTest)
		self)

	run = method (
		result = TestResult clone
		self runWithResult(result)
		result)

	runWithResult = method(aResult,
		tests foreach(i, v,
			v runResult(aResult)))
)

TestCase = Object clone do (

	type = "TestCase"

	testSelector = ""
	description = ""

	buildSuiteFromLocalSelectors = method (
		buildSuiteFromMethods(self localSelectors))

	buildSuiteFromMethods = method(selectors,
		selectors injectInto(TestSuite clone setName(type),
				block(suite, selector, suite addTest(self withSelector(selector)))))

	localSelectors = method (
		self slotNames select(block(each, each beginsWith("test"))))

	runCase = method (
		self perform(testSelector))

	runSuite = method (
		result = TestResult clone
		test = self buildSuiteFromLocalSelectors
		test runWithResult(result)
		result printOn(stdout))

	runResult = method (aResult,
		aResult runCase(self))

	selector = method (testSelector)

	setDescription = method (aDescription,
		self description = aDescription)

	setTestSelector = method (aSelector,
		self testSelector = aSelector
		return self)

	withSelector = method (aSelector,
		self clone setTestSelector(aSelector))


	assert = method (aCondition,
		// sender doMessage(_aCondition)
		// it seems delayed evaluation is not implemented yet
		assertDescription(aCondition, "assertion failed"))

	assertDescription = method (aCondition, aDescription,
		if (aCondition,
			Nil,
			raiseException(TestResult failure, aDescription)))

	assertEqual = method (expected, was,
		assertDescription(expected == was,
			"expected: " + expected asString + "  but was: " + was asString))

	assertNotEqual = method (notExpected, was,
		if (notExpected == was,
			assertDescription(Nil,
				"not expected: " + notExpected asString)))
)

IOUnit = Object clone do (
	TestCase	= TestCase
	TestResult	= TestResult
	TestSuite	= TestSuite
)

// testing Io

TestObject = Object clone do (
	type = "TestObject"

	parent = Object
	
	init = method (
		self parent = self parent clone)
)

IoTest = Object clone do (
	type = "IoTest"
	parent = TestCase

	testEmpty = method (Nil)
	testEQ = method (assert(Object == Object))
	testNE = method (assert(1 != 2))

	testCloningParent = method (
		assertNotEqual(TestObject clone parent, TestObject parent))

	testCloningObject = method (
		assertNotEqual(Object clone, Object clone))

	testTwoLists = method (
		assertNotEqual(List clone add(10), List clone))
)

IoTest runSuite
