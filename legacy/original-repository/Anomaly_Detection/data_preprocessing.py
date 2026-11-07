import pandas as pd
import logging
from sklearn.preprocessing import LabelEncoder, StandardScaler
from sklearn.model_selection import train_test_split
from data_acquisition import load_car_evaluation_data  # Import data loading function

# Configure logging
logging.basicConfig(level=logging.INFO, format='%(asctime)s - %(levelname)s - %(message)s')

def create_time_features(df):
    """
    Creates time-related features from the datetime index.

    Args:
        df (pd.DataFrame): The DataFrame with a datetime index.

    Returns:
        pd.DataFrame: The DataFrame with added time features.
    """
    logging.info("Creating time-related features...")
    df['hour'] = df.index.hour
    df['dayofweek'] = df.index.dayofweek
    df['month'] = df.index.month
    return df

def create_lag_features(df, target_column, lags=):
    """
    Creates lag features for the specified target column.

    Args:
        df (pd.DataFrame): The DataFrame with the target column.
        target_column (str): The name of the target column.
        lags (list): A list of lag values to create.

    Returns:
        pd.DataFrame: The DataFrame with added lag features.
    """
    logging.info("Creating lag features...")
    for lag in lags:
        df[f'{target_column}_lag_{lag}'] = df[target_column].shift(lag)
    return df

def scale_features(df, target_column):
    """
    Scales numerical features using StandardScaler.

    Args:
        df (pd.DataFrame): The DataFrame with numerical features.
        target_column (str): The name of the target column to exclude from scaling.

    Returns:
        pd.DataFrame: The DataFrame with scaled features.
    """
    logging.info("Scaling features...")
    features_to_scale = [col for col in df.columns if col != target_column]
    scaler = StandardScaler()
    df[features_to_scale] = scaler.fit_transform(df[features_to_scale])
    return df

def preprocess_data(df, target_column):
    """
    Preprocesses the data by creating time features, lag features, and scaling.

    Args:
        df (pd.DataFrame): The DataFrame to preprocess.
        target_column (str): The name of the target column.

    Returns:
        pd.DataFrame: The preprocessed DataFrame.
    """
    logging.info("Preprocessing data...")
    df = create_time_features(df)
    df = create_lag_features(df, target_column)
    df = df.dropna()  # Drop rows with NaN values due to lag features
    df = scale_features(df, target_column)
    logging.info("Data preprocessing completed.")
    return df

if __name__ == "__main__":
    transport_data = load_public_transport_data()
    if transport_data is not None:
        target_column = 'Count'  # Replace with the actual target column name
        preprocessed_data = preprocess_data(transport_data.copy(), target_column)
        logging.info(f"Preprocessed data shape: {preprocessed_data.shape}")
        logging.info(f"Preprocessed data columns: {preprocessed_data.columns.tolist()}")
        logging.info("First 5 rows of preprocessed data:")
        logging.info(preprocessed_data.head())
    else:
        logging.error("Data preprocessing failed due to data loading error.")

# Configure logging
logging.basicConfig(level=logging.INFO, format='%(asctime)s - %(levelname)s - %(message)s')

def encode_categorical_columns(df: pd.DataFrame) -> pd.DataFrame:
    """
    Encodes categorical columns in the DataFrame using LabelEncoder.

    Args:
        df (pd.DataFrame): The DataFrame to process.

    Returns:
        pd.DataFrame: The DataFrame with encoded categorical columns.
    """
    logging.info("Encoding categorical columns...")
    label_encoders = {}
    for column in df.columns:
        if df[column].dtype == 'object':
            label_encoders[column] = LabelEncoder()
            df[column] = label_encoders[column].fit_transform(df[column])
            logging.info(f"Encoded column: {column}")
    return df

def scale_numerical_columns(df: pd.DataFrame) -> pd.DataFrame:
    """
    Scales numerical columns in the DataFrame using StandardScaler.

    Args:
        df (pd.DataFrame): The DataFrame to process.

    Returns:
        pd.DataFrame: The DataFrame with scaled numerical columns.
    """
    logging.info("Scaling numerical columns...")
    scaler = StandardScaler()
    numerical_cols = df.select_dtypes(include=['number']).columns
    df[numerical_cols] = scaler.fit_transform(df[numerical_cols])
    logging.info("Numerical columns scaled.")
    return df

def preprocess_data(df: pd.DataFrame) -> tuple:
    """
    Preprocesses the data by encoding categorical columns, scaling numerical columns,
    and splitting the data into training and testing sets.

    Args:
        df (pd.DataFrame): The DataFrame to preprocess.

    Returns:
        tuple: A tuple containing X_train, X_test, y_train, y_test.
    """
    logging.info("Preprocessing data...")
    df = encode_categorical_columns(df)
    df = scale_numerical_columns(df)
    X = df.drop('class', axis=1)  # Features
    y = df['class']  # Target variable (anomaly labels)
    X_train, X_test, y_train, y_test = train_test_split(X, y, test_size=0.2, random_state=42)
    logging.info("Data preprocessing completed.")
    return X_train, X_test, y_train, y_test

if __name__ == "__main__":
    car_data = load_car_evaluation_data()
    if car_data is not None:
        X_train, X_test, y_train, y_test = preprocess_data(car_data.copy()) #copy the data, so that the original dataframe is not changed.
        logging.info(f"X_train shape: {X_train.shape}")
        logging.info(f"X_test shape: {X_test.shape}")
        logging.info(f"y_train shape: {y_train.shape}")
        logging.info(f"y_test shape: {y_test.shape}")
    else:
        logging.error("Data preprocessing failed due to data loading error.")
